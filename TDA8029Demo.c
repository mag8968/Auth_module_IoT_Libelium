/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                     Copyright (c) NXP Semiconductors
//
//         All rights are reserved. Reproduction in whole or in part is
//        prohibited without the written consent of the copyright owner.
//      NXP reserves the right to make changes without notice at any time.
//     NXP makes no warranty, expressed, implied or statutory, including but
//   not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party patent,
//     copyright or trademark. NXP must not be liable for any loss or damage
//                            arising from its use.
///////////////////////////////////////////////////////////////////////////////////////////////// */

/*! 
TDA Automatic test
*
*/
#include <stdio.h>
#include "SerialComm/SerialComm.h"
#include "TDA8029.h"

char TDA_Menu(void);

void main (void)
{
  char ans;
  char ret;

  // COM1 is used for the demonstration. To change it, refer to the function
  printf ("**********************************************************\n");
  printf ("***  TDA8029 Demonstration                             ***\n");
  printf ("**********************************************************\n");
  printf ("***                                                    ***\n");
  printf ("***  Connect a TDA8029 on COM1                         ***\n");
  printf ("***  And Press ENTER                                   ***\n");
  printf ("***                                                    ***\n");
  printf ("**********************************************************\n");


  fflush(stdin);
  ans = (char) getchar();
  do
  {
	  ret = TDA8029_CheckPluggedDevice();
	  if (ret != TDA8029_OK)
	  {
		  printf("*************************************************\n");
		  printf("*  Cannot find a TDA8029 on this port           *\n");
		  printf("*  Board is absent or COM Port is not available *\n");
		  printf("*************************************************\n");
		  printf("> Press ENTER"); 
		  fflush(stdin);
		  ans = (char) getchar();
	  }
  } while (ret != TDA8029_OK);

  if (ret == 0)
  {
	  // Run Main menu that calls the different Test Suites
	  do
	  {
		  ans = TDA_Menu();
	  }while(ans != 'x');
  }
}

char TDA_Menu(void)
{
	unsigned char answer;
	printf("*************************************************\n");
	printf("*            TDA8029Demo: Main Menu             *\n");
	printf("*************************************************\n");
	printf("*  Which test do you want to start ?            *\n");
	printf("*      1 - Check Mask Version                   *\n");
	printf("*      2 - Get Reader Status                    *\n");
	printf("*      3 - Check Card presence                  *\n");
	printf("*      4 - Activate Card                        *\n");
	printf("*      5 - Send an APDU                         *\n");
	printf("*      x - Exit	                                *\n");
	printf("*************************************************\n");
	printf("Your choice: ");
	fflush(stdin);
	answer = (char) getchar();

	if ((answer > '0') && (answer < '6'))
		TDA8029_TestStep(answer);

	return answer;
}
