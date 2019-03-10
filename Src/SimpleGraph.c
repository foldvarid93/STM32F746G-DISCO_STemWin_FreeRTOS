#include "DIALOG.h"

/*********************************************************************
 *
 *       Externals
 *
 **********************************************************************
 */

/*********************************************************************
 *
 *       Defines
 *
 **********************************************************************
 */
#define ID_WINDOW_0     (GUI_ID_USER + 0x00)
#define ID_GRAPH_0      (GUI_ID_USER + 0x01)

#define MESSAGE_STARTSTOP  (WM_USER + 0x00)

GRAPH_DATA_Handle PhaseDataA;
GRAPH_DATA_Handle PhaseDataB;
GRAPH_DATA_Handle PhaseDataC;
int NewPhaseDataA;
int NewPhaseDataB;
int NewPhaseDataC;
int AngleA = 0;
int AngleB = 0x555;
int AngleC = 0xAAA;
static int Stop;
/*********************************************************************
 *
 *       Static data
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       _aDialogCreate
 */
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
		{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 480, 272, 0, 0x0,0 },
		{ GRAPH_CreateIndirect, "Graph", ID_GRAPH_0, 5, 5, 470,	262, 0, 0x0, 0 },
// USER START (Optionally insert additional widgets)
// USER END
		};

/*********************************************************************
 *
 *       Static code
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       _cbGraph
 */
static void _cbGraph(WM_MESSAGE * pMsg) {
	GUI_PID_STATE * pState;
	static int Pressed;
	WM_HWIN hWin;

	switch (pMsg->MsgId) {
	case WM_TOUCH:
		pState = (GUI_PID_STATE *) pMsg->Data.p;
		if (pState) {
			if (pState->Pressed) {
				Pressed = 1;
			} else {
				if (Pressed) {
					Pressed = 0;
					//
					// If released send start stop message to the parent
					//
					hWin = WM_GetParent(pMsg->hWin);
					WM_SendMessageNoPara(hWin, MESSAGE_STARTSTOP);
				}
			}
		}
		break;
	default:
		GRAPH_Callback(pMsg);
		break;
	}
}

/*********************************************************************
 *
 *       _cbDialog
 */
static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int NumItems;

//  static WM_HTIMER hTimer;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		//
		// Initialization of 'Graph'
		//
		hItem = WM_GetDialogItem(pMsg->hWin, ID_GRAPH_0);
		//
		// Set a callback, it manages touch on the graph, if it gets touched it stops, another touch and it starts
		//
		WM_SetCallback(hItem, _cbGraph);
		//
		// Make the grid visible
		//
		GRAPH_SetGridVis(hItem, 1);
		//
		// Get x size, used as max num items
		//
		NumItems = WM_GetWindowSizeX(hItem);
		//
		// Create two data items, one for sin, one for cos
		//
		PhaseDataA = GRAPH_DATA_YT_Create(GUI_YELLOW, NumItems, NULL, 0);
		PhaseDataB = GRAPH_DATA_YT_Create(GUI_WHITE, NumItems, NULL, 0);
		PhaseDataC = GRAPH_DATA_YT_Create(GUI_LIGHTCYAN, NumItems, NULL, 0);
		//
		// Attach them to the GRAPH
		//
		GRAPH_AttachData(hItem, PhaseDataA);
		GRAPH_AttachData(hItem, PhaseDataB);
		GRAPH_AttachData(hItem, PhaseDataC);
		//
		// Create a timer which updates the GRAPH
		//
		//hTimer = WM_CreateTimer(pMsg->hWin, 0, 10, 0);
		break;
		/*  case WM_TIMER:
		 //
		 // Depending on Stop, restart the graph
		 //
		 if (Stop == 0) {
		 WM_RestartTimer(hTimer, 10);
		 }
		 //
		 // Calculate new values for the graph and add them to the data items, the GRAPH gets updated automatically
		 //
		 NewPhaseDataA =GUI_sin(AngleA)/10;
		 NewPhaseDataB =GUI_sin(AngleB)/10;
		 NewPhaseDataC =GUI_sin(AngleC)/10;
		 AngleA+=10;
		 AngleB+=10;
		 AngleC+=10;
		 if(AngleA==0xfff){
		 AngleA=0;
		 AngleB=0x555;
		 AngleC=0xAAA;
		 }
		 GRAPH_DATA_YT_AddValue(PhaseDataA, NewPhaseDataA+131);
		 GRAPH_DATA_YT_AddValue(PhaseDataB, NewPhaseDataB+131);
		 GRAPH_DATA_YT_AddValue(PhaseDataC, NewPhaseDataC+131);
		 break;*/
	case MESSAGE_STARTSTOP:
		//
		// Start stop message send by the GRAPH
		//
		Stop ^= 1;
		/*    if (Stop == 0) {
		 WM_RestartTimer(hTimer, 10);
		 }*/
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}

/*********************************************************************
 *
 *       _cbBk
 */
static void _cbBk(WM_MESSAGE * pMsg) {
	switch (pMsg->MsgId) {
	case WM_PAINT:
		GUI_SetBkColor(GUI_BLACK);
		GUI_Clear();
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}

/*********************************************************************
 *
 *       Public code
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       MainTask
 */
void MainTask(void) {
	WM_SetCallback(WM_HBKWIN, _cbBk);
	GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
void NewData(void)
{
		if(Stop){}
		else{
		NewPhaseDataA = GUI_sin(AngleA) / 10;
		NewPhaseDataB = GUI_sin(AngleB) / 10;
		NewPhaseDataC = GUI_sin(AngleC) / 10;
		AngleA += 10;
		AngleB += 10;
		AngleC += 10;
		if (AngleA == 0xfff) {
			AngleA = 0;
			AngleB = 0x555;
			AngleC = 0xAAA;
		}
		GRAPH_DATA_YT_AddValue(PhaseDataA, NewPhaseDataA + 131);
		GRAPH_DATA_YT_AddValue(PhaseDataB, NewPhaseDataB + 131);
		GRAPH_DATA_YT_AddValue(PhaseDataC, NewPhaseDataC + 131);
		}
}
