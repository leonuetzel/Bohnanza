#include "../Inc/ITN/consolePlayer.hpp"
#include <iostream>





/*****************************************************************************/
/*                    Globals and Static Initialization					 						 */
/*****************************************************************************/

std::map<Bohnanza::e_bean, std::string> beanNames =
{
	{Bohnanza::e_bean::NONE,				"Nothing"},
	{Bohnanza::e_bean::GARTENBOHNE,	"Gartenbohne"},
	{Bohnanza::e_bean::ROTE_BOHNE,	"Rote Bohne"},
	{Bohnanza::e_bean::AUGENBOHNE,	"Augenbohne"},
	{Bohnanza::e_bean::SOJABOHNE,		"Sojabohne"},
	{Bohnanza::e_bean::BRECHBOHNE,	"Brechbohne"},
	{Bohnanza::e_bean::SAUBOHNE,		"Saubohne"},
	{Bohnanza::e_bean::FEUERBOHNE,	"Feuerbohne"},
	{Bohnanza::e_bean::BLAUE_BOHNE,	"Blaue Bohne"}
};



/*****************************************************************************/
/*                      						Private	  			 						 						 */
/*****************************************************************************/

uint8 ConsolePlayer::readInputNumber()
{
	while(1)
	{
		std::string input;
		std::cin >> input;
		try
		{
			uint8 number = std::stoi(input);
			return(number);
		}
		catch(...)
		{
			std::cout << std::endl << "Invalid Input. Please enter a Number: ";
		}
	}
}


Bohnanza::e_bean ConsolePlayer::readInputBean()
{
	std::cout << "Translation Table: " << std::endl;
	for(auto& i: beanNames)
	{
		std::cout << (uint8) i.first << " - " << i.second << std::endl;
	}
	std::cout << "Please enter the Number of the Bean you want to plant: ";
	
	while(1)
	{
		const uint8 number = readInputNumber();
		if(number > 0 && number <= 8)
		{
			return((Bohnanza::e_bean) number);
		}
		else
		{
			std::cout << "Invalid Input. Please enter a Number between 1 and 8: ";
		}
	}
}







void ConsolePlayer::informAboutHand(const std::vector<Bohnanza::e_bean>& hand)
{
	if(hand.empty() == true)
	{
		std::cout << "You have no Beans in your Hand." << std::endl;
		return;
	}
	
	std::cout << "You have the following Beans in your Hand: " << std::endl;
	uint32 index = 0;
	for(auto& i: hand)
	{
		std::cout << index << " - " << beanNames[i] << std::endl;
		index++;
	}
	std::cout << std::endl;
}


void ConsolePlayer::informAboutFields(const std::vector<std::pair<Bohnanza::e_bean, uint8>>& field)
{
	std::cout << "Your Fields are growing the following Beans: " << std::endl;
	uint32 index = 0;
	for(auto& i: field)
	{
		if(i.second == 0)
		{
			std::cout << "Field " << index << ": " << " Grows nothing" << std::endl;
			index++;
			continue;
		}
		
		std::cout << "Field " << index << ": Grows " << (uint32) i.second << " " << beanNames[i.first];
		if(i.second > 1)
		{
			std::cout << "n";
		}
		std::cout << std::endl;
		index++;
	}
	std::cout << std::endl;
}


void ConsolePlayer::informAboutTradeBeans(const std::vector<Bohnanza::e_bean>& tradeBeans)
{
	std::cout << "The Trade Beans are: " << std::endl;
	uint32 index = 0;
	for(auto& i: tradeBeans)
	{
		std::cout << index << " - " << beanNames[i] << std::endl;
		index++;
	}
	std::cout << std::endl;
}



/*****************************************************************************/
/*                      						Public	  			 						 						 */
/*****************************************************************************/

ConsolePlayer::ConsolePlayer(const std::string& name)
	:	m_name(name)
{
	std::cout << "Created Console Player " << m_name << std::endl;
}


ConsolePlayer::~ConsolePlayer()
{
	std::cout << "Destroyed Console Player " << m_name << std::endl;
}







const std::string& ConsolePlayer::getName() const
{
	return(m_name);
}







void ConsolePlayer::action_phase1_plant(Bohnanza& bohnanza)
{
	std::cout << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------" << std::endl;
	std::cout << "Its your Turn, " << m_name << std::endl;
	std::cout << std::endl;
	
	
	//	Inform the Player about his Fields
	const uint8 playerID = bohnanza.getOwnPlayerID(*this);
	std::vector<std::pair<Bohnanza::e_bean, uint8>> fields;
	for(uint8 i = 0; i < 2; ++i)
	{
		fields.push_back(bohnanza.getField(playerID, i));
	}
	if(bohnanza.ownsThirdField(playerID) == true)
	{
		fields.push_back(bohnanza.getField(playerID, 2));
	}
	informAboutFields(fields);
	
	
	//	Inform the Player about his Hand
	const std::vector<Bohnanza::e_bean>& hand = bohnanza.getHand(playerID);
	informAboutHand(hand);
	
	
	//	Check if the Player has Beans left to plant
	if(hand.size() == 0)
	{
		std::cout << "You have no Beans left to plant - Moving to Phase 2..." << std::endl;
		return;
	}
	
	
	//	Actions:
	//	p: Plant a Bean from the Hand on a Field
	//	h: Harvest a Field
	//	f: Finish Planting (only possible if the Player has planted at least one Bean)
	bool hasPlantedAtLeastOneBean = false;
	bool placedSecondBean = false;
	std::cout << std::endl;
	std::cout << "Phase 1: Planting - You must plant a " << beanNames[hand[0]] << std::endl << std::endl;
	
	std::cout << "Choose an Action: " << std::endl;
	std::cout << "p: Plant a Bean from the Hand on a Field" << std::endl;
	std::cout << "h: Harvest a Field" << std::endl;
	std::cout << "f: Finish Planting (only possible if at least one Bean has been planted)" << std::endl;
	
	while(1)
	{
		std::cout << "Choose an Action: ";
		std::string input;
		std::cin >> input;
		
		if(input == "p")
		{
			if(hand.size() == 0)
			{
				std::cout << "You have no Beans left to plant." << std::endl;
				continue;
			}
			
			if(placedSecondBean == true)
			{
				std::cout << "You have already planted 2 Beans from your Hand.." << std::endl;
				continue;
			}
			
			
			//	Plant a Bean from the Hand on a Field
			std::cout << "Choose a Field to plant the " << beanNames[bohnanza.getHand(playerID)[0]] << ": ";
			const uint8 fieldID = readInputNumber();
			if(bohnanza.plantFromHand(fieldID) == ITN::OK)
			{
				std::cout << "Bean planted successfully." << std::endl;
				if(hasPlantedAtLeastOneBean == true)
				{
					placedSecondBean = true;
				}
				hasPlantedAtLeastOneBean = true;
			}
			else
			{
				std::cout << "Planting failed. Please try again." << std::endl;
			}
		}
		
		if(input == "h")
		{
			//	Harvest a Field
			std::cout << "Choose a Field to harvest: ";
			const uint8 fieldID = readInputNumber();
			if(bohnanza.harvest(playerID, fieldID) == ITN::OK)
			{
				std::cout << "Field harvested successfully." << std::endl;
			}
			else
			{
				std::cout << "Harvesting failed. Please try again." << std::endl;
			}
		}
		
		if(input == "f")
		{
			//	Finish Planting
			if(hasPlantedAtLeastOneBean == true)
			{
				std::cout << "Moving to Phase 2..." << std::endl;
				return;
			}
			else
			{
				std::cout << "You have not planted any Beans yet. Please plant at least one Bean." << std::endl;
			}
		}
	}
}


void ConsolePlayer::action_phase2_trade(Bohnanza& bohnanza)
{
	std::cout << std::endl;
	std::cout << "Phase 2: Trading" << std::endl;
	//	Read Field Information
	const uint8 playerID = bohnanza.getOwnPlayerID(*this);
	std::vector<std::pair<Bohnanza::e_bean, uint8>> fields;
	for(uint8 i = 0; i < 2; ++i)
	{
		fields.push_back(bohnanza.getField(playerID, i));
	}
	if(bohnanza.ownsThirdField(playerID) == true)
	{
		fields.push_back(bohnanza.getField(playerID, 2));
	}
	
	
	//	Read Trade Beans Information
	std::vector<Bohnanza::e_bean> tradeBeans;
	for(uint8 i = 0; i < 2; ++i)
	{
		tradeBeans.push_back(bohnanza.getTradeBean(i));
	}
	informAboutTradeBeans(tradeBeans);
	
	
	//	Actions:
	//	p: Plant a Trade Bean on a Field
	//	t: Trade with another Player
	//	h: Harvest a Field
	//	f: Finish Trading (only possible if no Trade Beans are left)
	std::cout << "Choose an Action: " << std::endl;
	std::cout << "p: Plant a Trade Bean on a Field" << std::endl;
	std::cout << "t: Trade with another Player" << std::endl;
	std::cout << "h: Harvest a Field" << std::endl;
	std::cout << "f: Finish Trading (only possible if no Trade Beans are left)" << std::endl;
	
	while(1)
	{
		std::cout << "Choose an Action: ";
		std::string input;
		std::cin >> input;
		
		if(input == "p")
		{
			//	Plant a Trade Bean on a Field
			std::cout << "Choose a Trade Bean to plant: ";
			const uint8 tradeBeanIndex = readInputNumber();
			std::cout << "Choose a Field to plant it: ";
			const uint8 fieldID = readInputNumber();
			if(bohnanza.plantTradeBean(fieldID, tradeBeanIndex) == ITN::OK)
			{
				std::cout << "Trade Bean planted successfully." << std::endl;
			}
			else
			{
				std::cout << "Planting failed." << std::endl;
			}
		}
		
		if(input == "t")
		{
			//	Actions:
			//	l: Look for Trade Requests from other Players
			//	c: Create a Trade Request
			std::cout << "Choose an Action: " << std::endl;
			std::cout << "l: Look for Trade Requests from other Players" << std::endl;
			std::cout << "c: Create a Trade Request" << std::endl;
			
			while(1)
			{
				std::cout << "Choose an Action: ";
				std::string input;
				std::cin >> input;
				
				if(input == "l")
				{
					//	Look for Trade Requests from other Players
					bohnanza.checkForTradeRequestsFromOtherPlayers();
					break;
				}
				
				if(input == "c")
				{
					//	Create a Trade Request
					std::cout << "Choose a Player to trade with: ";
					const uint8 playerIDTradePartner = readInputNumber();
					std::cout << "Choose the Beans you want to offer: " << std::endl;
					for(uint32 i = 1; i < 9; ++i)
					{
						std::cout << i << " - " << beanNames[(Bohnanza::e_bean) i] << std::endl;
					}
					std::map<Bohnanza::e_bean, uint8> beansOffered;
					while(1)
					{
						std::cout << "Choose a Bean to offer (finish by entering 0): ";
						const uint8 beanID = readInputNumber();
						if(beanID == 0)
						{
							break;
						}
						if(beanID < 9)
						{
							beansOffered[(Bohnanza::e_bean) beanID]++;
						}
					}
					std::cout << "Choose the Beans you want to request: " << std::endl;
					std::map<Bohnanza::e_bean, uint8> beansRequested;
					while(1)
					{
						std::cout << "Choose a Bean to request (finish by entering 0): ";
						const uint8 beanID = readInputNumber();
						if(beanID == 0)
						{
							break;
						}
						if(beanID < 9)
						{
							beansRequested[(Bohnanza::e_bean) beanID]++;
						}
					}
					
					if(bohnanza.trade(playerID, playerIDTradePartner, beansOffered, beansRequested) == ITN::OK)
					{
						std::cout << "-----------------------------------------------------------------------------------------" << std::endl;
						std::cout << getName() << ", your Trade with " << bohnanza.getPlayer(playerIDTradePartner).getName() << " has been successful." << std::endl;
					}
					else
					{
						std::cout << "-----------------------------------------------------------------------------------------" << std::endl;
						std::cout << getName() << ", your Trade with " << bohnanza.getPlayer(playerIDTradePartner).getName() << " failed." << std::endl;
					}
					std::cout << "You have " << (uint32) bohnanza.getNumberOfRemainingTradeRequests(playerID) << " Trade Request";
					if(bohnanza.getNumberOfRemainingTradeRequests(playerID) != 1)
					{
						std::cout << "s";
					}
					std::cout << " left for this Turn." << std::endl;
					break;
				}
			}
		}
		
		if(input == "h")
		{
			//	Harvest a Field
			std::cout << "Choose a Field to harvest: ";
			const uint8 fieldID = readInputNumber();
			if(bohnanza.harvest(playerID, fieldID) == ITN::OK)
			{
				std::cout << "Field harvested successfully." << std::endl;
			}
			else
			{
				std::cout << "Harvesting failed." << std::endl;
			}
		}
		
		if(input == "f")
		{
			//	Finish Trading
			if(bohnanza.getTradeBean(0) == Bohnanza::e_bean::NONE && bohnanza.getTradeBean(1) == Bohnanza::e_bean::NONE)
			{
				std::cout << "Moving to Phase 3..." << std::endl;
				return;
			}
			else
			{
				std::cout << "You still have Trade Beans left. Please plant them on your Fields or trade them with other Players." << std::endl;
			}
		}
	}
}


void ConsolePlayer::action_phase2_tradeWithPlayerOnTurn(Bohnanza& bohnanza, uint8 playerIDonTurn)
{
	//	To Do: Implement a Strategy for Trading with the Player on Turn
	std::cout << "Another Player is checking on you (Player" << m_name << ") for Trading Offers - automatically offering nothing." << std::endl;
}







void ConsolePlayer::notification_phase1_plant(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean)
{
	
}


void ConsolePlayer::notification_phase2_tradeHappened(uint8 playerIDrequesting, uint8 playerIDTradePartner, const std::map<Bohnanza::e_bean, uint8>& beansOffered, const std::map<Bohnanza::e_bean, uint8>& beansRequested)
{
	
}


void ConsolePlayer::notification_phase2_plantBeanFromTradePartner(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean)
{
	
}


void ConsolePlayer::notification_phase2_plantBeanFromTradeBeans(uint8 playerID, Bohnanza::e_bean bean)
{
	
}


void ConsolePlayer::notification_phase3_draw(uint8 playerID, uint8 numberOfCards)
{
	
}


void ConsolePlayer::notification_harvested(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean, uint8 numberOfBeans)
{
	
}


void ConsolePlayer::notification_boughtThirdField(uint8 playerID, const std::map<Bohnanza::e_bean, uint8>& coins)
{
	
}







ITN::feedback ConsolePlayer::reaction_tradeOffer(Bohnanza& bohnanza, uint8 playerID, const std::map<Bohnanza::e_bean, uint8>& beansOffered, const std::map<Bohnanza::e_bean, uint8>& beansRequested)
{
	//	Inform the Player about the Trade Offer
	Player& player = bohnanza.getPlayer(playerID);
	std::cout << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------" << std::endl;
	std::cout << player.getName() << " wants to trade with you (" << getName() << ")" << std::endl;
	std::cout << "He offers the following Beans: " << std::endl;
	for(auto& i: beansOffered)
	{
		std::cout << (uint32) i.second << " " << beanNames[i.first];
		if(i.second > 1)
		{
			std::cout << "n";
		}
		std::cout << std::endl;
	}
	std::cout << "He requests the following Beans: " << std::endl;
	for(auto& i: beansRequested)
	{
		std::cout << (uint32) i.second << " " << beanNames[i.first];
		if(i.second > 1)
		{
			std::cout << "n";
		}
		std::cout << std::endl;
	}
	
	
	//	Ask the Player if he agrees to the Trade
	std::cout << "Do you agree to the Trade? (y/n): ";
	while(1)
	{
		std::string input;
		std::cin >> input;
		if(input == "y")
		{
			return(ITN::OK);
		}
		if(input == "n")
		{
			return(ITN::FAIL);
		}
		std::cout << std::endl << "Invalid Input. Please enter 'y' or 'n': ";
	}
}


uint8 ConsolePlayer::reaction_chooseBeanToGiveForTrade(Bohnanza& bohnanza, Bohnanza::e_bean bean)
{
	//	Choose the Bean to give for the Trade
	std::cout << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------" << std::endl;
	std::cout << getName() << ", your recent Trade is signed now." << std::endl;
	std::cout << "You have to give a " << beanNames[bean] << " for this Trade." << std::endl;
	std::cout << "Choose a Bean to give: " << std::endl;
	
	const uint8 playerID = bohnanza.getOwnPlayerID(*this);
	const uint8 playerIDonTurn = bohnanza.getPlayerIDonTurn();
	const std::vector<Bohnanza::e_bean>& hand = bohnanza.getHand(bohnanza.getOwnPlayerID(*this));
	for(uint32 i = 0; i < 7; ++i)
	{
		if(i < 5)
		{
			if(i < hand.size())
			{
				if(hand[i] == bean)
				{
					std::cout << i << " - " << beanNames[hand[i]] << " from your Hand" << std::endl;
				}
			}
		}
		else
		{
			if(playerID == playerIDonTurn)
			{
				if(bohnanza.getTradeBean(i - 5) == bean)
				{
					std::cout << i << " - " << beanNames[bohnanza.getTradeBean(i - 5)] << " from the Trade Beans" << std::endl;
				}
			}
		}
	}
	
	while(1)
	{
		std::cout << "Choose a Bean to give: ";
		const uint8 beanPosition = readInputNumber();
		if(beanPosition < 8)
		{
			if(beanPosition < 5)
			{
				if(beanPosition < hand.size())
				{
					return(beanPosition);
				}
			}
			else
			{
				return(beanPosition);
			}
		}
		std::cout << "Invalid Input. Please enter a Number between 0 and 7: ";
	}
}


void ConsolePlayer::reaction_plantBeansFromTrade(Bohnanza& bohnanza, const std::map<Bohnanza::e_bean, uint8>& beans)
{
	//	Plant the Beans from the Trade
	std::cout << std::endl;
	std::cout << "-----------------------------------------------------------------------------------------" << std::endl;
	const uint8 playerID = bohnanza.getOwnPlayerID(*this);
	std::cout << getName() << ", you have to plant the following Beans from the Trade: " << std::endl;
	for(auto& i: beans)
	{
		std::cout << (uint32) i.second << " " << beanNames[i.first];
		if(i.second > 1)
		{
			std::cout << "n";
		}
		std::cout << std::endl;
	}
	
	
	//	Actions:
	//	p: Plant a Bean from the Trade on a Field
	//	h: Harvest a Field
	std::cout << std::endl;
	std::cout << "Actions: " << std::endl;
	std::cout << "p: Plant a Bean from the Trade on a Field" << std::endl;
	std::cout << "h: Harvest a Field" << std::endl;
	
	while(1)
	{
		std::cout << "Choose an Action: ";
		std::string input;
		std::cin >> input;
		
		if(input == "p")
		{
			//	Plant a Bean from the Trade on a Field
			//	Choose a Field to plant the Bean on
			std::cout << "Choose a Field to plant the Bean on: ";
			const uint8 fieldID = readInputNumber();
			
			
			//	Choose a Bean to plant
			uint32 index = 0;
			for(auto& i: beans)
			{
				std::cout << index << " - " << beanNames[i.first] << std::endl;
				index++;
			}
			std::cout << "Choose a Bean to plant: ";
			const uint8 beanIndex = readInputNumber();
			index = 0;
			Bohnanza::e_bean bean;
			for(auto& i: beans)
			{
				if(index == beanIndex)
				{
					bean = i.first;
					break;
				}
				index++;
			}
			if(bohnanza.plantBeansFromTrade(playerID, fieldID, bean) == ITN::OK)
			{
				std::cout << "Bean planted successfully." << std::endl;
				return;
			}
			else
			{
				std::cout << "Planting failed." << std::endl;
				return;
			}
		}
		
		if(input == "h")
		{
			//	Harvest a Field
			std::cout << "Choose a Field to harvest: ";
			const uint8 fieldID = readInputNumber();
			if(bohnanza.harvest(playerID, fieldID) == ITN::OK)
			{
				std::cout << "Field harvested successfully." << std::endl;
			}
			else
			{
				std::cout << "Harvesting failed." << std::endl;
			}
		}
	}
}