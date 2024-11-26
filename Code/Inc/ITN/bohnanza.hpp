#pragma once

#include <ITN/defines.hpp>
#include <vector>
#include <map>





class Player;

class Bohnanza
{
	public:
		
		enum class e_bean
		{
			NONE				= 0,
			GARTENBOHNE	= 1,
			ROTE_BOHNE	= 2,
			AUGENBOHNE	= 3,
			SOJABOHNE		= 4,
			BRECHBOHNE	= 5,
			SAUBOHNE		= 6,
			FEUERBOHNE	= 7,
			BLAUE_BOHNE	= 8
		};
		
		static constexpr uint8 numberOfTradeRequestsPerPhase2 = 15;
		static const std::map<e_bean, uint8> distribution;
		static const std::map<e_bean, std::map<uint8, uint8>> priceTable;
		
		
		
		
		
	private:
		
		//	Static Member
		static constexpr uint8 maximumNumberOfPlayers = 3;
		static constexpr uint8 minimumNumberOfPlayers = 5;
		
		typedef struct
		{
			Player* player;
			std::vector<e_bean> hand;
			std::pair<e_bean, uint8> field[3];
			std::vector<e_bean> coins;
			bool ownsThirdField;
		}s_player;
		
		
		//	Non-static Member
		bool m_started;
		bool m_finishingAfterThisPlayer;
		bool m_finished;
		std::vector<s_player> m_players;
		std::vector<e_bean> m_deck;
		std::vector<e_bean> m_compost;
		uint8 m_shuffleCounter;
		uint8 m_playerIDonTurn;
		uint8 m_phase1_plantCounter;
		e_bean m_phase2_tradeBeans[2];
		std::vector<std::map<e_bean, uint8>> m_phase2_beansTradedButNotYetPlanted;
		std::vector<uint8> m_tradeRequestsAvailable;
		
		
		//	Constructor and Destructor
		
		
		
		//	Member Functions
		void shuffle();
		
		
		//	Friends
		
		
		
		
		
		
	public:
		
		Bohnanza();
		~Bohnanza();
		
		ITN::feedback registerPlayer(Player& player);
		ITN::feedback run();
		
		
		//	Player Actions
		ITN::feedback plantFromHand(uint8 fieldID);
		ITN::feedback harvest(uint8 playerID, uint8 fieldID);
		ITN::feedback buyThirdField(uint8 playerID, const std::map<e_bean, uint8>& coins);
		ITN::feedback trade(uint8 playerIDrequesting, uint8 playerIDTradePartner, const std::map<e_bean, uint8>& beansOffered, std::map<e_bean, uint8>& beansRequested);
		void checkForTradeRequestsFromOtherPlayers();
		ITN::feedback plantTradeBean(uint8 fieldID, uint8 tradeBeanIndex);
		ITN::feedback plantBeansFromTrade(uint8 playerID, uint8 fieldID, e_bean bean);
		
		
		//	Getter
		uint8 getNumberOfPlayers() const;
		uint8 getOwnPlayerID(Player& player) const;
		uint8 getPlayerIDonTurn() const;
		Player& getPlayer(uint8 playerID) const;
		
		bool ownsThirdField(uint8 playerID) const;
		uint8 getNumberOfRemainingTradeRequests(uint8 playerID) const;
		
		const std::vector<e_bean>& getHand(uint8 playerID) const;
		const std::pair<e_bean, uint8>& getField(uint8 playerID, uint8 fieldID) const;
		uint8 getNumberOfCoins(uint8 playerID) const;
		e_bean getTradeBean(uint8 index) const;
};



/*****************************************************************************/
/*                    Globals and Static Initialization					 						 */
/*****************************************************************************/





/*****************************************************************************/
/*                      						Private	  			 						 						 */
/*****************************************************************************/





/*****************************************************************************/
/*                      					Protected	  			 						 						 */
/*****************************************************************************/





/*****************************************************************************/
/*                      						Public	  			 						 						 */
/*****************************************************************************/

