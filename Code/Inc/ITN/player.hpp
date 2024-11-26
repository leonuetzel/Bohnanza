#pragma once

#include <ITN/defines.hpp>
#include <string>
#include "bohnanza.hpp"





class Player
{
	public:
		
		
		
		
		
		
		
	private:
		
		//	Static Member
		
		
		
		//	Non-static Member
		
		
		
		//	Constructor and Destructor
		
		
		
		//	Member Functions
		
		
		
		//	Friends
		
		
		
		
		
		
	public:
		
		virtual const std::string& getName() const = 0;
		
		//	Player Actions when its his Turn - these Functions will be called by the Game
		virtual void action_phase1_plant(Bohnanza& bohnanza) = 0;
		virtual void action_phase2_trade(Bohnanza& bohnanza) = 0;
		virtual void action_phase2_tradeWithPlayerOnTurn(Bohnanza& bohnanza, uint8 playerIDonTurn) = 0;
		
		
		//	Notifiers for Game Actions executed by any Player - these Functions will be called by the Game
		virtual void notification_phase1_plant(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean) = 0;
		virtual void notification_phase2_tradeHappened(uint8 playerIDrequesting, uint8 playerIDTradePartner, const std::map<Bohnanza::e_bean, uint8>& beansOffered, const std::map<Bohnanza::e_bean, uint8>& beansRequested) = 0;
		virtual void notification_phase2_plantBeanFromTradePartner(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean) = 0;
		virtual void notification_phase2_plantBeanFromTradeBeans(uint8 playerID, Bohnanza::e_bean bean) = 0;
		virtual void notification_phase3_draw(uint8 playerID, uint8 numberOfCards) = 0;
		virtual void notification_harvested(uint8 playerID, uint8 fieldID, Bohnanza::e_bean bean, uint8 numberOfBeans) = 0;
		virtual void notification_boughtThirdField(uint8 playerID, const std::map<Bohnanza::e_bean, uint8>& coins) = 0;
		
		
		//	Trade Actions - these Functions will be called by other Players (mediated through the Game) to trade with this Player
		virtual ITN::feedback reaction_tradeOffer(Bohnanza& bohnanza, uint8 playerID, const std::map<Bohnanza::e_bean, uint8>& beansOffered, const std::map<Bohnanza::e_bean, uint8>& beansRequested) = 0;
		virtual uint8 reaction_chooseBeanToGiveForTrade(Bohnanza& bohnanza, Bohnanza::e_bean bean) = 0;	//	Bean IDs 0-4 are the Beans in the Hand, 5 and 6 are the Trade Beans if available
		virtual void reaction_plantBeansFromTrade(Bohnanza& bohnanza, const std::map<Bohnanza::e_bean, uint8>& beans) = 0;
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

