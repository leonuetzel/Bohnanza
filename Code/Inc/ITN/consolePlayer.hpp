#pragma once

#include "player.hpp"
#include <string>





class ConsolePlayer: public Player
{
	public:
		
		
		
		
		
		
		
	private:
		
		//	Static Member
		
		
		
		//	Non-static Member
		const std::string m_name;
		
		
		//	Constructor and Destructor
		
		
		
		//	Member Functions
		uint8 readInputNumber();
		Bohnanza::e_bean readInputBean();
		
		void informAboutHand(const std::vector<Bohnanza::e_bean>& hand);
		void informAboutFields(const std::vector<std::pair<Bohnanza::e_bean, uint8>>& field);
		void informAboutTradeBeans(const std::vector<Bohnanza::e_bean>& tradeBeans);
		
		
		//	Friends
		
		
		
		
		
		
	public:
		
		ConsolePlayer(const std::string& name);
		~ConsolePlayer();
		
		const std::string& getName() const override;
		
		//	Player Actions when its his Turn - these Functions will be called by the Game
		void action_phase1_plant(Bohnanza& bohnanza) override;
		void action_phase2_trade(Bohnanza& bohnanza) override;
		void action_phase2_tradeWithPlayerOnTurn(Bohnanza& bohnanza, uint8 playerIDonTurn) override;
		
		
		//	Notifiers for Game Actions executed by any Player - these Functions will be called by the Game
		void notification_gameStarted(uint8 playerID, const std::vector<std::pair<Bohnanza::e_bean, uint8>>& fields, const std::vector<Bohnanza::e_bean>& hand) override;
		void notification_phase1_plant(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean) override;
		void notification_phase2_tradeHappened(uint8 playerIDrequesting, uint8 playerIDTradePartner, const std::map<Bohnanza::e_bean, uint8>& beansOffered, const std::map<Bohnanza::e_bean, uint8>& beansRequested) override;
		void notification_phase2_plantBeanFromTradePartner(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean) override;
		void notification_phase2_plantBeanFromTradeBeans(uint8 playerID, Bohnanza::e_bean bean) override;
		void notification_phase3_draw(uint8 playerID, uint8 numberOfCards) override;
		void notification_harvested(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean, uint8 numberOfBeans) override;
		void notification_boughtThirdField(uint8 playerID, const std::map<Bohnanza::e_bean, uint8>& coins) override;
		
		
		//	Trade Actions - these Functions will be called by other Players (mediated through the Game) to trade with this Player
		ITN::feedback reaction_tradeOffer(Bohnanza& bohnanza, uint8 playerID, const std::map<Bohnanza::e_bean, uint8>& beansOffered, const std::map<Bohnanza::e_bean, uint8>& beansRequested) override;
		uint8 reaction_chooseBeanToGiveForTrade(Bohnanza& bohnanza, Bohnanza::e_bean bean) override;	//	Bean IDs 0-4 are the Beans in the Hand, 5 and 6 are the Trade Beans if available
		void reaction_plantBeansFromTrade(Bohnanza& bohnanza, const std::map<Bohnanza::e_bean, uint8>& beans) override;
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

