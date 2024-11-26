#include <ITN/main.hpp>




int main()
{
	Bohnanza bohnanza;
	
	ConsolePlayer player1("Player 0");
	ConsolePlayer player2("Player 1");
	ConsolePlayer player3("Player 2");
	
	bohnanza.registerPlayer(player1);
	bohnanza.registerPlayer(player2);
	bohnanza.registerPlayer(player3);
	
	bohnanza.run();
	
	
	//	Give the User some Time to read the Output and close the Console
	system("pause");
	return(ITN::OK);
}