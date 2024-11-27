#include "../Inc/ITN/bohnanza.hpp"
#include "../Inc/ITN/player.hpp"
#include <algorithm>
#include <random>
#include <chrono>





/*****************************************************************************/
/*                    Globals and Static Initialization					 						 */
/*****************************************************************************/

const std::map<Bohnanza::e_bean, uint8> Bohnanza::distribution =
{
	{e_bean::GARTENBOHNE,	 6},
	{e_bean::ROTE_BOHNE,	 8},
	{e_bean::AUGENBOHNE,	10},
	{e_bean::SOJABOHNE,		12},
	{e_bean::BRECHBOHNE,	14},
	{e_bean::SAUBOHNE,		16},
	{e_bean::FEUERBOHNE,	18},
	{e_bean::BLAUE_BOHNE,	20}
};

const std::map<Bohnanza::e_bean, std::map<uint8, uint8>> Bohnanza::priceTable =
{
	{e_bean::GARTENBOHNE,	{        {2, 2}, {3, 3}        }},
	{e_bean::ROTE_BOHNE,	{{1, 2}, {2, 3}, {3, 4}, {4,  5}}},
	{e_bean::AUGENBOHNE,	{{1, 2}, {2, 4}, {3, 5}, {4,  6}}},
	{e_bean::SOJABOHNE,		{{1, 2}, {2, 4}, {3, 6}, {4,  7}}},
	{e_bean::BRECHBOHNE,	{{1, 3}, {2, 5}, {3, 6}, {4,  7}}},
	{e_bean::SAUBOHNE,		{{1, 3}, {2, 5}, {3, 7}, {4,  8}}},
	{e_bean::FEUERBOHNE,	{{1, 3}, {2, 6}, {3, 8}, {4,  9}}},
	{e_bean::BLAUE_BOHNE,	{{1, 4}, {2, 6}, {3, 8}, {4, 10}}}
};



/*****************************************************************************/
/*                      						Private	  			 						 						 */
/*****************************************************************************/

void Bohnanza::shuffle()
{
	//	Shuffle the Deck
	uint32 seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rng = std::default_random_engine{seed};
	std::shuffle(std::begin(m_deck), std::end(m_deck), rng);
}



/*****************************************************************************/
/*                      						Public	  			 						 						 */
/*****************************************************************************/

Bohnanza::Bohnanza()
{
	shuffle();
}


Bohnanza::~Bohnanza()
{
	
}







ITN::feedback Bohnanza::registerPlayer(Player& player)
{
	if(m_players.size() >= maximumNumberOfPlayers || m_started == true || m_finished == true)
	{
		return(ITN::FAIL);
	}
	s_player playerStruct;
	playerStruct.player = &player;
	playerStruct.hand.clear();
	for(auto& i: playerStruct.field)
	{
		i.first = e_bean::NONE;
		i.second = 0;
	}
	playerStruct.coins.clear();
	playerStruct.ownsThirdField = false;
	
	m_players.push_back(playerStruct);
	return(ITN::OK);
}


ITN::feedback Bohnanza::run()
{
	//	Check if the Game can be started
	if(m_players.size() < 3 || m_players.size() > 5 || m_started == true || m_finished == true)
	{
		return(ITN::FAIL);
	}
	
	
	//	Setup the Game State
	for(auto& i: m_players)
	{
		i.hand.clear();
		i.coins.clear();
		i.ownsThirdField = false;
	}
	m_finishingAfterThisPlayer = false;
	m_compost.clear();
	m_shuffleCounter = 0;
	m_playerIDonTurn = 0;
	m_phase1_plantCounter = 0;
	m_phase2_tradeBeans[0] = e_bean::NONE;
	m_phase2_tradeBeans[1] = e_bean::NONE;
	m_phase2_beansTradedButNotYetPlanted.clear();
	m_phase2_beansTradedButNotYetPlanted.resize(m_players.size());
	
	
	//	Create an unshuffled Deck of Beans according to the Distribution
	m_deck.clear();
	for(auto& bean: distribution)
	{
		for(uint8 i = 0; i < bean.second; ++i)
		{
			m_deck.push_back(bean.first);
		}
	}
	
	
	//	Shuffle the Deck
	shuffle();
	
	
	//	Hand out 5 Cards to each Player
	for(auto& i: m_players)
	{
		for(uint8 j = 0; j < 5; ++j)
		{
			i.hand.push_back(m_deck[0]);
			m_deck.erase(m_deck.begin());
		}
	}
	
	
	//	Game Start
	m_started = true;
	
	
	//	Notify all Players
	for(uint32 i = 0; i < m_players.size(); ++i)
	{
		Player& player = *m_players[i].player;
		std::vector<std::pair<e_bean, uint8>> fields;
		for(uint8 j = 0; j < 2; ++j)
		{
			fields.push_back(m_players[i].field[j]);
		}
		if(m_players[i].ownsThirdField == true)
		{
			fields.push_back(m_players[i].field[2]);
		}
		player.notification_gameStarted(i, fields, m_players[i].hand);
	}
	
	
	while(1)
	{
		for(auto& i: m_players)
		{
			Player& player = *i.player;
			
			//	Phase 1: Plant Beans
			//	One Bean is mandatory to plant, the second Bean is optional
			//	Save the Number of Beans in the Hand before the Phase
			m_phase1_plantCounter = 0;
			player.action_phase1_plant(*this);
			
			
			//	Phase 2: Trade Beans
			//	The Player draws 2 Beans from the Deck (visible to all Players) and has to:
			//	- plant them on his own Fields or
			//	- trade them with other Players so that they plant them
			//	The drawn Beans must be planted on a Field at the End of this Phase
			//	During the whole Phase, the Player can trade Beans from his Hand with Beans from other Player's Hands
			//	Beans received through Trading must immediately be planted on a Field the receiving Player owns
			//	This Phase ends when the Player on Turn decides (by returning from the Function).
			m_tradeRequestsAvailable.clear();
			m_tradeRequestsAvailable.resize(m_players.size());
			for(auto& i: m_tradeRequestsAvailable)
			{
				i = numberOfTradeRequestsPerPhase2;
			}
			
			
			for(auto& j: m_phase2_tradeBeans)
			{
				j = m_deck[0];
				m_deck.erase(m_deck.begin());
				
				//	Check if the Deck is empty
				if(m_deck.empty() == true)
				{
					//	Check the Shuffle Counter - if the Compost got shuffled 2 times already, the Game ends immediately
					if(m_shuffleCounter >= 2)
					{
						m_finishingAfterThisPlayer = true;
					}
					
					
					//	There are no more Beans in the Deck - the Player is left with the Beans he has drawn
					break;
				}
			}
			
			while(m_phase2_tradeBeans[0] != e_bean::NONE || m_phase2_tradeBeans[1] != e_bean::NONE)
			{
				player.action_phase2_trade(*this);
			}
			
			
			//	Phase 3: Draw Cards until the Hand has 5 Cards
			const uint8 numberOfCardsToDraw = 5 - i.hand.size();
			for(uint8 j = 0; j < numberOfCardsToDraw; ++j)
			{
				//	Draw a Card from the Deck
				i.hand.push_back(m_deck[0]);
				m_deck.erase(m_deck.begin());
				
				
				//	Check if the Deck is empty
				if(m_deck.empty() == true)
				{
					//	Check the Shuffle Counter - if the Compost got shuffled 2 times already, the Game ends immediately
					if(m_shuffleCounter >= 2)
					{
						m_finished = true;
						return(ITN::OK);
					}
					
					
					//	Shuffle the Compost and put it back to the Deck
					m_deck = m_compost;
					m_compost.clear();
					shuffle();
					
					
					//	Increment the Shuffle Counter
					m_shuffleCounter++;
				}
			}
			
			
			//	Check for the End of the Game
			if(m_finishingAfterThisPlayer == true)
			{
				m_finished = true;
				return(ITN::OK);
			}
			
			
			//	Increment the Player on Turn
			m_playerIDonTurn = (m_playerIDonTurn + 1) % m_players.size();
		}
	}
}







ITN::feedback Bohnanza::plantFromHand(uint8 fieldID)
{
	//	Check if the FieldID is valid and that the Player is allowed to plant a Bean (maximum 2 per Turn)
	if(fieldID >= 3 || m_phase1_plantCounter >= 2)
	{
		return(ITN::FAIL);
	}
	
	s_player& player = m_players[m_playerIDonTurn];
	if(fieldID == 2 && player.ownsThirdField == false)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Hand has any Beans left to plant
	if(player.hand.empty())
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Field is empty or already contains Beans of the same Type
	const e_bean bean = player.hand[0];
	std::pair<e_bean, uint8>& field = player.field[fieldID];
	if(field.first == e_bean::NONE || field.first == bean)
	{
		//	Plant the Bean
		field.first = bean;
		field.second++;
		
		
		//	Remove the Bean from the Hand
		player.hand.erase(player.hand.begin());
		
		
		//	Increment the Plant Counter
		m_phase1_plantCounter++;
		
		
		//	Notify all Players
		for(auto& i: m_players)
		{
			Player& player = *i.player;
			player.notification_phase1_plant(m_playerIDonTurn, fieldID, bean);
		}
		return(ITN::OK);
	}
	return(ITN::FAIL);
}


ITN::feedback Bohnanza::harvest(uint8 playerID, uint8 field)
{
	//	Check the PlayerID
	if(playerID >= m_players.size())
	{
		return(ITN::FAIL);
	}
	
	
	s_player& player = m_players[playerID];
	std::pair<e_bean, uint8>& fieldToHarvest = player.field[field];
	if(fieldToHarvest.first == e_bean::NONE || fieldToHarvest.second == 0)
	{
		return(ITN::OK);
	}
	
	
	//	Harvesting a Field is only permitted if
	//	- the Field contains at least 2 Beans or
	//	- the Field contains only 1 Bean and there is no other Field with more than 1 Bean
	if(fieldToHarvest.second < 2)
	{
		bool onlyFieldsWithMaximumOneBean = true;
		for(auto& i: player.field)
		{
			if(i.second > 1)
			{
				onlyFieldsWithMaximumOneBean = false;
				break;
			}
		}
		
		
		//	There are Fields with more than 1 Bean - Harvesting is not permitted
		if(onlyFieldsWithMaximumOneBean == false)
		{
			return(ITN::FAIL);
		}
	}
	
	
	//	Harvesting is permitted - harvest the Field
	//	Find out how many Coins the Player receives for the Harvest
	const uint8 numberOfBeansGrown = fieldToHarvest.second;
	const std::map<uint8, uint8>& priceTableOfBean = Bohnanza::priceTable.at(fieldToHarvest.first);
	
	uint8 coins = 0;
	uint8 beansRequiredForThatPrice = 0;
	for(auto& i: priceTableOfBean)
	{
		if(numberOfBeansGrown >= i.second)
		{
			const uint8 coinsNew = i.first;
			if(coinsNew > coins)
			{
				coins = coinsNew;
				beansRequiredForThatPrice = i.second;
			}
		}
	}
	
	
	//	Remove the Beans from the Field
	fieldToHarvest.first = e_bean::NONE;
	fieldToHarvest.second = 0;
	
	
	//	Throw the Beans (except the ones that will be put to the Coins-Stack and re-used as Coins) to the Compost
	for(uint8 i = 0; i < numberOfBeansGrown - beansRequiredForThatPrice; ++i)
	{
		m_compost.push_back(fieldToHarvest.first);
	}
	
	
	//	Put the Beans that will be re-used as Coins to the Coins-Stack
	for(uint8 i = 0; i < beansRequiredForThatPrice; ++i)
	{
		player.coins.push_back(fieldToHarvest.first);
	}
	
	
	//	Notify all Players
	for(auto& i: m_players)
	{
		Player& player = *i.player;
		player.notification_harvested(playerID, field, fieldToHarvest.first, numberOfBeansGrown);
	}
	
	
	return(ITN::OK);
}


ITN::feedback Bohnanza::buyThirdField(uint8 playerID, const std::map<e_bean, uint8>& coins)
{
	//	Check the PlayerID
	if(playerID >= m_players.size())
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Player has enough Coins to buy the Third Field or if he already owns it
	s_player& player = m_players[playerID];
	if(player.ownsThirdField == true || player.coins.size() < 3)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Player has the required Coins in his Coin-Stack
	std::map<e_bean, uint8> coinsAvailable;
	for(auto& i: player.coins)
	{
		coinsAvailable[i]++;
	}
	
	for(auto& i: coins)
	{
		if(coinsAvailable[i.first] < i.second)
		{
			return(ITN::FAIL);
		}
	}
	
	
	//	Remove the Coins from the Coin-Stack and place these Coins as Beans on the Compost
	for(auto& i: coins)
	{
		for(uint8 j = 0; j < i.second; ++j)
		{
			player.coins.erase(std::find(player.coins.begin(), player.coins.end(), i.first));
			m_compost.push_back(i.first);
		}
	}
	
	
	//	Give the Player the Ownership of the Third Field
	player.ownsThirdField = true;
	
	
	//	Notify all Players
	for(auto& i: m_players)
	{
		Player& player = *i.player;
		player.notification_boughtThirdField(playerID, coins);
	}
	
	
	return(ITN::OK);
}


ITN::feedback Bohnanza::trade(uint8 playerIDrequesting, uint8 playerIDTradePartner, const std::map<e_bean, uint8>& beansOffered, std::map<e_bean, uint8>& beansRequested)
{
	//	Check the PlayerID
	if(playerIDrequesting >= m_players.size() || playerIDTradePartner >= m_players.size() )
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Player requesting has remaining Trade Requests available
	if(m_tradeRequestsAvailable[playerIDrequesting] <= 0)
	{
		return(ITN::FAIL);
	}
	
	
	//	Decrease the Number of Trade Requests available for the Player requesting
	m_tradeRequestsAvailable[playerIDrequesting]--;
	
	
	//	The Player requesting wants to trade with himself - this is not permitted
	if(playerIDrequesting == playerIDTradePartner)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check for valid Bean Types and Amounts
	for(auto& i: beansOffered)
	{
		if(i.first == e_bean::NONE || i.second == 0)
		{
			//	The Player wants to offer a Bean that is not valid or the Amount is 0
			return(ITN::FAIL);
		}
		
		const uint8 maximumNumberOfBeans = distribution.at(i.first);
		if(i.second > maximumNumberOfBeans)
		{
			//	The Player wants to offer more Beans than are available in the Game
			return(ITN::FAIL);
		}
	}
	
	
	//	Check if the Player requesting has the Beans he wants to offer
	s_player& playerRequesting = m_players[playerIDrequesting];
	std::map<e_bean, uint8> beansAvailable;
	for(auto& i: playerRequesting.hand)
	{
		beansAvailable[i]++;
	}
	if(playerIDrequesting == m_playerIDonTurn)
	{
		//	Special Case: The Player on Turn has drawn 2 Beans from the Deck that he can trade with
		beansAvailable[m_phase2_tradeBeans[0]]++;
		beansAvailable[m_phase2_tradeBeans[1]]++;
	}
	
	for(auto& i: beansOffered)
	{
		if(beansAvailable[i.first] < i.second)
		{
			//	The Player does not have the Beans he wants to offer
			return(ITN::FAIL);
		}
	}
	
	
	//	Check if the Trade Partner has the Beans that are requested
	s_player& playerTradePartner = m_players[playerIDTradePartner];
	std::map<e_bean, uint8> beansAvailableTradePartner;
	for(auto& i: playerTradePartner.hand)
	{
		beansAvailableTradePartner[i]++;
	}
	if(playerIDTradePartner == m_playerIDonTurn)
	{
		//	Special Case: The Player on Turn has drawn 2 Beans from the Deck that he can trade with
		beansAvailableTradePartner[m_phase2_tradeBeans[0]]++;
		beansAvailableTradePartner[m_phase2_tradeBeans[1]]++;
	}
	
	for(auto& i: beansRequested)
	{
		if(beansAvailableTradePartner[i.first] < i.second)
		{
			//	The Trade Partner does not have the Beans that are requested
			return(ITN::FAIL);
		}
	}
	
	
	//	Ask the Trade Partner if he agrees to the Trade
	Player& tradePartner = *playerTradePartner.player;
	const ITN::feedback tradeAgreed = tradePartner.reaction_tradeOffer(*this, playerIDrequesting, beansOffered, beansRequested);
	if(tradeAgreed == ITN::FAIL)
	{
		//	The Trade Partner does not agree to the Trade
		return(ITN::FAIL);
	}
	
	
	//	Ask the Players, which exact Beans from which Position in their Hands/Trade Beans they want to give for the Trade
	std::vector<uint8> beansOfferedToRemove;
	for(auto& i: beansOffered)
	{
		for(uint8 j = 0; j < i.second; ++j)
		{
			//	Ask the Player that offered the Beans which Bean from which Position in his Hand/Trade Cards he wants to give
			//	Bean IDs 0-4 are the Beans in the Hand, 5 and 6 are the Trade Beans if available
			Player& player = *playerRequesting.player;
			const uint8 beanPosition = player.reaction_chooseBeanToGiveForTrade(*this, i.first);
			
			
			//	Check for valid Bean Position
			if(beanPosition > 6)
			{
				return(ITN::FAIL);
			}
			if(beanPosition > 4 && playerIDrequesting != m_playerIDonTurn)
			{
				//	The Player on Turn can only offer the Beans in his Hand
				return(ITN::FAIL);
			}
			if(beanPosition >= playerRequesting.hand.size() && beanPosition < 5)
			{
				//	The Player does not have enough Beans in his Hand so the told Bean Position is empty
				return(ITN::FAIL);
			}
			
			
			//	Check for duplicate Bean Positions
			if(std::find(beansOfferedToRemove.begin(), beansOfferedToRemove.end(), beanPosition) != beansOfferedToRemove.end())
			{
				//	The Player has already offered that Bean Position
				return(ITN::FAIL);
			}
			
			
			//	Add the Bean Positions to the List of Beans to remove
			beansOfferedToRemove.push_back(beanPosition);
		}
	}
	
	std::vector<uint8> beansRequestedToRemove;
	for(auto& i: beansRequested)
	{
		for(uint8 j = 0; j < i.second; ++j)
		{
			//	Ask the Trade Partner which Bean from which Position in his Hand/Trade Cards he wants to give
			//	Bean IDs 0-4 are the Beans in the Hand, 5 and 6 are the Trade Beans if available
			Player& player = *playerTradePartner.player;
			const uint8 beanPosition = player.reaction_chooseBeanToGiveForTrade(*this, i.first);
			
			
			//	Check for valid Bean Position
			if(beanPosition > 6)
			{
				return(ITN::FAIL);
			}
			if(beanPosition > 4 && playerIDTradePartner != m_playerIDonTurn)
			{
				//	The Player on Turn can only offer the Beans in his Hand
				return(ITN::FAIL);
			}
			if(beanPosition >= playerTradePartner.hand.size() && beanPosition < 5)
			{
				//	The Player does not have enough Beans in his Hand so the told Bean Position is empty
				return(ITN::FAIL);
			}
			
			
			//	Check for duplicate Bean Positions
			if(std::find(beansRequestedToRemove.begin(), beansRequestedToRemove.end(), beanPosition) != beansRequestedToRemove.end())
			{
				//	The Player has already offered that Bean Position
				return(ITN::FAIL);
			}
			
			
			//	Add the Bean Positions to the List of Beans to remove
			beansRequestedToRemove.push_back(beanPosition);
		}
	}
	
	
	//	Check that the told Beans are of the correct Type
	{
		std::map<e_bean, uint8> beansOfferedToCheck = beansOffered;
		for(auto& i: beansOfferedToRemove)
		{
			e_bean bean = e_bean::NONE;
			if(i < 5)
			{
				bean = playerRequesting.hand[i];
			}
			else
			{
				bean = m_phase2_tradeBeans[i - 5];
			}
			
			if(beansOfferedToCheck.find(bean) == beansOfferedToCheck.end())
			{
				//	The Player has told a Bean that was not part of the Trade Offer
				return(ITN::FAIL);
			}
			beansOfferedToCheck[bean]--;
		}
		if(beansOfferedToCheck.size() != 0)
		{
			for(auto& i: beansOfferedToCheck)
			{
				if(i.second != 0)
				{
					//	The Player has not told all Beans that were part of the Trade Offer
					return(ITN::FAIL);
				}
			}
		}
	}
	{
		std::map<e_bean, uint8> beansRequestedToCheck = beansRequested;
		for(auto& i: beansRequestedToRemove)
		{
			e_bean bean = e_bean::NONE;
			if(i < 5)
			{
				bean = playerTradePartner.hand[i];
			}
			else
			{
				bean = m_phase2_tradeBeans[i - 5];
			}
			
			if(beansRequestedToCheck.find(bean) == beansRequestedToCheck.end())
			{
				//	The Player has told a Bean that was not part of the Trade Request
				return(ITN::FAIL);
			}
			beansRequestedToCheck[bean]--;
		}
		if(beansRequestedToCheck.size() != 0)
		{
			for(auto& i: beansRequestedToCheck)
			{
				if(i.second != 0)
				{
					//	The Player has not told all Beans that were part of the Trade Request
					return(ITN::FAIL);
				}
			}
		}
	}
	
	
	//	--------------------------------------------------------------------------------------------------------------------------
	//	From this Point on, the Trade is valid and will be executed - so dont return FAIL anymore because Changes are already made
	//	--------------------------------------------------------------------------------------------------------------------------
	
	
	//	Remove these Beans from the Hands/Trade Beans of the Players and place them to the Queue of Beans that must be planted after the Trade
	std::sort(beansOfferedToRemove.begin(), beansOfferedToRemove.end(), std::greater<uint8>());
	std::sort(beansRequestedToRemove.begin(), beansRequestedToRemove.end(), std::greater<uint8>());
	
	for(auto& i: beansOfferedToRemove)
	{
		if(i < 5)
		{
			//	The Bean is in the Hand
			const e_bean bean = playerRequesting.hand[i];
			playerRequesting.hand.erase(playerRequesting.hand.begin() + i);
			m_phase2_beansTradedButNotYetPlanted[playerIDTradePartner][bean]++;
		}
		else
		{
			//	The Bean is a Trade Bean
			const e_bean bean = m_phase2_tradeBeans[i - 5];
			m_phase2_tradeBeans[i - 5] = e_bean::NONE;
			m_phase2_beansTradedButNotYetPlanted[playerIDTradePartner][bean]++;
		}
	}
	
	for(auto& i: beansRequestedToRemove)
	{
		if(i < 5)
		{
			//	The Bean is in the Hand
			const e_bean bean = playerTradePartner.hand[i];
			playerTradePartner.hand.erase(playerTradePartner.hand.begin() + i);
			m_phase2_beansTradedButNotYetPlanted[playerIDrequesting][bean]++;
		}
		else
		{
			//	The Bean is a Trade Bean
			const e_bean bean = m_phase2_tradeBeans[i - 5];
			m_phase2_tradeBeans[i - 5] = e_bean::NONE;
			m_phase2_beansTradedButNotYetPlanted[playerIDrequesting][bean]++;
		}
	}
	
	
	//	Notify all Players
	for(auto& i: m_players)
	{
		Player& player = *i.player;
		player.notification_phase2_tradeHappened(playerIDrequesting, playerIDTradePartner, beansOffered, beansRequested);
	}
	
	
	//	Plant the traded Beans
	while(1)
	{
		bool allBeansPlanted = true;
		for(auto& i: m_phase2_beansTradedButNotYetPlanted[playerIDrequesting])
		{
			if(i.second > 0)
			{
				allBeansPlanted = false;
				m_players[playerIDrequesting].player->reaction_plantBeansFromTrade(*this, m_phase2_beansTradedButNotYetPlanted[playerIDrequesting]);
			}
		}
		if(allBeansPlanted == true)
		{
			m_phase2_beansTradedButNotYetPlanted[playerIDrequesting].clear();
			break;
		}
	}
	while(m_phase2_beansTradedButNotYetPlanted[playerIDTradePartner].size() > 0)
	{
		bool allBeansPlanted = true;
		for(auto& i: m_phase2_beansTradedButNotYetPlanted[playerIDTradePartner])
		{
			if(i.second > 0)
			{
				allBeansPlanted = false;
				m_players[playerIDTradePartner].player->reaction_plantBeansFromTrade(*this, m_phase2_beansTradedButNotYetPlanted[playerIDTradePartner]);
			}
		}
		if(allBeansPlanted == true)
		{
			m_phase2_beansTradedButNotYetPlanted[playerIDTradePartner].clear();
			break;
		}
	}
	
	
	return(ITN::OK);
}


void Bohnanza::checkForTradeRequestsFromOtherPlayers()
{
	if(m_tradeRequestsAvailable[m_playerIDonTurn] <= 0)
	{
		return;
	}
	
	for(uint32 i = 1; i < m_players.size(); ++i)
	{
		if(i != m_playerIDonTurn)
		{
			Player& player = *m_players[i].player;
			player.action_phase2_tradeWithPlayerOnTurn(*this, m_playerIDonTurn);
		}
	}
	m_tradeRequestsAvailable[m_playerIDonTurn]--;
}


ITN::feedback Bohnanza::plantTradeBean(uint8 fieldID, uint8 tradeBeanIndex)
{
	//	Check if the tradeBeanIndex is valid
	if(tradeBeanIndex >= 2)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the FieldID is valid
	if(fieldID >= 3)
	{
		return(ITN::FAIL);
	}
	if(fieldID == 2 && m_players[m_playerIDonTurn].ownsThirdField == false)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if this Bean is available for planting
	const e_bean bean = m_phase2_tradeBeans[tradeBeanIndex];
	if(bean == e_bean::NONE)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Field is empty or already contains Beans of the same Type
	s_player& player = m_players[m_playerIDonTurn];
	std::pair<e_bean, uint8>& field = player.field[fieldID];
	if(field.first != e_bean::NONE && field.first != bean)
	{
		return(ITN::FAIL);
	}
	
	
	//	Plant the Bean
	field.first = bean;
	field.second++;
	
	
	//	Remove the Bean from the Trade Beans
	m_phase2_tradeBeans[tradeBeanIndex] = e_bean::NONE;
	
	
	//	Notify all Players
	for(auto& i: m_players)
	{
		Player& player = *i.player;
		player.notification_phase2_plantBeanFromTradeBeans(m_playerIDonTurn, bean);
	}
	
	
	return(ITN::OK);
}


ITN::feedback Bohnanza::plantBeansFromTrade(uint8 playerID, uint8 fieldID, e_bean bean)
{
	//	Check the PlayerID
	if(playerID >= m_players.size())
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the FieldID is valid
	if(fieldID >= 3)
	{
		return(ITN::FAIL);
	}
	if(fieldID == 2 && m_players[playerID].ownsThirdField == false)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Player has said Bean in his Queue of Beans that must be planted after a Trade
	const std::map<e_bean, uint8>& beansToPlant = m_phase2_beansTradedButNotYetPlanted[playerID];
	if(beansToPlant.find(bean) == beansToPlant.end())
	{
		return(ITN::FAIL);
	}
	if(beansToPlant.at(bean) <= 0)
	{
		return(ITN::FAIL);
	}
	
	
	//	Check if the Field is empty or already contains Beans of the same Type
	s_player& player = m_players[playerID];
	std::pair<e_bean, uint8>& field = player.field[fieldID];
	if(field.first != e_bean::NONE && field.first != bean)
	{
		return(ITN::FAIL);
	}
	
	
	//	Plant the Bean
	field.first = bean;
	field.second++;
	
	
	//	Remove the Bean from the Queue of Beans that must be planted after a Trade
	m_phase2_beansTradedButNotYetPlanted[playerID][bean]--;
	
	
	//	Notify all Players
	for(auto& i: m_players)
	{
		Player& player = *i.player;
		player.notification_phase2_plantBeanFromTradePartner(playerID, fieldID, bean);
	}
	
	
	return(ITN::OK);
}







uint8 Bohnanza::getNumberOfPlayers() const
{
	return(m_players.size());
}


uint8 Bohnanza::getOwnPlayerID(Player& player) const
{
	for(uint8 i = 0; i < m_players.size(); ++i)
	{
		if(m_players[i].player == &player)
		{
			return(i);
		}
	}
	return(m_players.size());
}


uint8 Bohnanza::getPlayerIDonTurn() const
{
	return(m_playerIDonTurn);
}


Player& Bohnanza::getPlayer(uint8 playerID) const
{
	if(playerID >= m_players.size())
	{
		return((Player&) (*(Player*) nullptr));
	}
	return(*m_players[playerID].player);
}







bool Bohnanza::ownsThirdField(uint8 playerID) const
{
	if(playerID >= m_players.size())
	{
		return(false);
	}
	return(m_players[playerID].ownsThirdField);
}


uint8 Bohnanza::getNumberOfRemainingTradeRequests(uint8 playerID) const
{
	if(playerID >= m_players.size())
	{
		return(0);
	}
	return(m_tradeRequestsAvailable[playerID]);
}







const std::vector<Bohnanza::e_bean>& Bohnanza::getHand(uint8 playerID) const
{
	if(playerID >= m_players.size())
	{
		return((std::vector<Bohnanza::e_bean>&) (*(std::vector<Bohnanza::e_bean>*) nullptr));
	}
	return(m_players[playerID].hand);
}


const std::pair<Bohnanza::e_bean, uint8>& Bohnanza::getField(uint8 playerID, uint8 fieldID) const
{
	if(playerID >= m_players.size())
	{
		return((std::pair<Bohnanza::e_bean, uint8>&) (*(std::pair<Bohnanza::e_bean, uint8>*) nullptr));
	}
	return(m_players[playerID].field[fieldID]);
}


uint8 Bohnanza::getNumberOfCoins(uint8 playerID) const
{
	if(playerID >= m_players.size())
	{
		return(0);
	}
	return(m_players[playerID].coins.size());
}


Bohnanza::e_bean Bohnanza::getTradeBean(uint8 index) const
{
	if(index >= 2)
	{
		return(e_bean::NONE);
	}
	return(m_phase2_tradeBeans[index]);
}