/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "MyEventReceiver.hpp"

#include <string>

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include "Utilities.hpp"

using namespace irr;

    MyEventReceiver::MyEventReceiver(irr::IrrlichtDevice* dev, SimulationModel* model, GUIMain* gui, JoystickSetup joystickSetup, std::vector<std::string>* logMessages) //Constructor
	{
		this->model = model; //Link to the model
		this->gui = gui; //Link to GUI (Not currently used, all comms through model)
		scrollBarPosSpeed = 0;
		scrollBarPosHeading = 0;

		//store device
		device = dev;

		//set up joystick if present, and inform user what's available
		dev->activateJoysticks(joystickInfo);

		//Tell user about joysticks via the log
		dev->getLogger()->log(""); //add a blank line
		std::string joystickInfoMessage = "Number of joysticks detected: ";
		joystickInfoMessage.append(std::string(core::stringc(joystickInfo.size()).c_str()));
		dev->getLogger()->log(joystickInfoMessage.c_str());
        for(unsigned int i = 0; i<joystickInfo.size(); i++) {
            //Print out name and number of each joystick
            joystickInfoMessage = "Joystick number: ";
            joystickInfoMessage.append(core::stringc(i).c_str());
            joystickInfoMessage.append(", Name: ");
            joystickInfoMessage.append(std::string(joystickInfo[i].Name.c_str()));
            dev->getLogger()->log(joystickInfoMessage.c_str());
        }
        dev->getLogger()->log(""); //add a blank line

		this->joystickSetup = joystickSetup;

		//Indicate that previous joystick information hasn't been initialised
		previousJoystickPort = INFINITY;
		previousJoystickStbd = INFINITY;
		previousJoystickRudder = INFINITY;
		previousJoystickBowThruster = INFINITY;
		previousJoystickSternThruster = INFINITY;

		this->logMessages = logMessages;

        //assume mouse buttons not pressed initially
        leftMouseDown = false;
        rightMouseDown = false;
	}

    bool MyEventReceiver::OnEvent(const SEvent& event)
	{


        //std::cout << "Any event in receiver" << std::endl;
        //From log
        if (event.EventType == EET_LOG_TEXT_EVENT) {
            //Store these in a global log.
            std::string eventText(event.LogEvent.Text);
            logMessages->push_back(eventText);
            return true;
        }

        //From mouse - keep track of button press state
        if (event.EventType == EET_MOUSE_INPUT_EVENT) {
            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN ) {leftMouseDown=true;}
            if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP ) {leftMouseDown=false;}
            if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN ) {
                rightMouseDown=true;
                //Force focus on right click
                gui::IGUIElement* overElement;
                overElement = device->getGUIEnvironment()->getRootGUIElement()->getElementFromPoint(core::position2d<s32>(event.MouseInput.X,event.MouseInput.Y));
                if (overElement) {
                    device->getGUIEnvironment()->setFocus(overElement);
                }
            }
            if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP ) {rightMouseDown=false;}
            model->setMouseDown(leftMouseDown || rightMouseDown); //Set if either mouse is down
        }

        if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
            if (event.GUIEvent.EventType==gui::EGET_SCROLL_BAR_CHANGED)
            {

               if (id == GUIMain::GUI_ID_HEADING_SCROLL_BAR)
                  {
                      scrollBarPosHeading = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                      model->setHeading(scrollBarPosHeading);
                  }

              if (id == GUIMain::GUI_ID_SPEED_SCROLL_BAR)
                  {
                        scrollBarPosSpeed = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        model->setSpeed(scrollBarPosSpeed);
                  }

              if (id == GUIMain::GUI_ID_STBD_SCROLL_BAR)
                  {
                        irr::f32 value = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0;  //Convert to from +-100 to +-1, and invert up/down
                        model->setStbdEngine(value);
                        //If right mouse button, set the other engine as well
                        if (rightMouseDown) {
                            model->setPortEngine(value);
                        }
                  }
              if (id == GUIMain::GUI_ID_PORT_SCROLL_BAR)
                  {
                        irr::f32 value = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/-100.0;  //Convert to from +-100 to +-1, and invert up/down
                        model->setPortEngine(value);
                        //If right mouse button, set the other engine as well
                        if (rightMouseDown) {
                            model->setStbdEngine(value);
                        }
                  }

// DEE debug this must be disabled becuase only the wheel is directly controlled
// disbaling mouse controlling the rudder directly change it to change wheel

              if (id == GUIMain::GUI_ID_RUDDER_SCROLL_BAR)
                  {
//                        model->setRudder(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }




// DEE capture the wheel
              if (id == GUIMain::GUI_ID_WHEEL_SCROLL_BAR)
                  {
                        model->setWheel(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
// DEE capture the wheel


            //DEAL WITH THRUSTER SCROLL BARS HERE - ALSO WITH JOYSTICK

              if (id == GUIMain::GUI_ID_BOWTHRUSTER_SCROLL_BAR) {
                  irr::f32 value = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/100.0;  //Convert to from +-100 to +-1
                  model->setBowThruster(value);
              }
              if (id == GUIMain::GUI_ID_STERNTHRUSTER_SCROLL_BAR) {
                  irr::f32 value = ((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/100.0;  //Convert to from +-100 to +-1
                  model->setSternThruster(value);
              }

              if (id == GUIMain::GUI_ID_RADAR_GAIN_SCROLL_BAR)
                  {
                        model->setRadarGain(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_CLUTTER_SCROLL_BAR)
                  {
                        model->setRadarClutter(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_RADAR_RAIN_SCROLL_BAR)
                  {
                        model->setRadarRain(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos());
                  }
              if (id == GUIMain::GUI_ID_WEATHER_SCROLL_BAR)
                  {
                        model->setWeather(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 0-120, weather 0-12
                  }
              if (id == GUIMain::GUI_ID_RAIN_SCROLL_BAR)
                  {
                        model->setRain(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 0-100, rain 0-10
                  }
              if (id == GUIMain::GUI_ID_VISIBILITY_SCROLL_BAR)
                  {
                        model->setVisibility(((gui::IGUIScrollBar*)event.GUIEvent.Caller)->getPos()/10.0); //Scroll bar 1-101, vis 0.1-10.1
                  }
            }

            if (event.GUIEvent.EventType==gui::EGET_MESSAGEBOX_OK) {
                if (id == GUIMain::GUI_ID_CLOSE_BOX) {
                    device->closeDevice(); //Confirm shutdown.
                }
            }

            if (event.GUIEvent.EventType==gui::EGET_BUTTON_CLICKED) {
                if (id == GUIMain::GUI_ID_START_BUTTON)
                {
                    model->setAccelerator(1.0);
                }

                if (id == GUIMain::GUI_ID_RADAR_INCREASE_BUTTON)
                {
                    model->increaseRadarRange();
                }

                if (id == GUIMain::GUI_ID_RADAR_DECREASE_BUTTON)
                {
                    model->decreaseRadarRange();
                }

                if (id == GUIMain::GUI_ID_BIG_RADAR_BUTTON)
                {
                    gui->setLargeRadar(true);
                    model->setRadarDisplayRadius(gui->getRadarPixelRadius());
                    gui->hide2dInterface();
                }

                if (id == GUIMain::GUI_ID_SMALL_RADAR_BUTTON)
                {
                    gui->setLargeRadar(false);
                    model->setRadarDisplayRadius(gui->getRadarPixelRadius());
                    gui->show2dInterface();
                }

                if (id == GUIMain::GUI_ID_SHOW_INTERFACE_BUTTON)
                {
                    gui->show2dInterface();
                }

                if (id == GUIMain::GUI_ID_HIDE_INTERFACE_BUTTON)
                {
                    gui->hide2dInterface();
                }

                if (id == GUIMain::GUI_ID_BINOS_INTERFACE_BUTTON)
                {
                    model->setZoom(((gui::IGUIButton*)event.GUIEvent.Caller)->isPressed());
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_LEFT_BUTTON)
                {
                    model->decreaseRadarEBLBrg();
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_RIGHT_BUTTON)
                {
                    model->increaseRadarEBLBrg();
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_UP_BUTTON)
                {
                    model->increaseRadarEBLRange();
                }

                if (id == GUIMain::GUI_ID_RADAR_EBL_DOWN_BUTTON)
                {
                    model->decreaseRadarEBLRange();
                }

                //Radar mode buttons
                if (id == GUIMain::GUI_ID_RADAR_NORTH_BUTTON)
                {
                    model->setRadarNorthUp();
                }
                if (id == GUIMain::GUI_ID_RADAR_COURSE_BUTTON)
                {
                    model->setRadarCourseUp();
                }
                if (id == GUIMain::GUI_ID_RADAR_HEAD_BUTTON)
                {
                    model->setRadarHeadUp();
                }

                if (id == GUIMain::GUI_ID_SHOW_LOG_BUTTON)
                {
                    gui->showLogWindow();
                }

            } //Button clicked

            if (event.GUIEvent.EventType == gui::EGET_COMBO_BOX_CHANGED) {

                if (id == GUIMain::GUI_ID_ARPA_TRUE_REL_BOX || id == GUIMain::GUI_ID_BIG_ARPA_TRUE_REL_BOX)
                {
                    s32 selected = ((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected();
                    if(selected == 0) {
                        model->setRadarARPATrue();
                    } else if (selected == 1) {
                        model->setRadarARPARel();
                    }

                    //Set both linked inputs - brute force
                    gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_TRUE_REL_BOX,true);
                    if(other!=0) {
                        ((gui::IGUIComboBox*)other)->setSelected(((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    }
                    other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_TRUE_REL_BOX,true);
                    if(other!=0) {
                        ((gui::IGUIComboBox*)other)->setSelected(((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                    }

                }

            }//Combo box

            if ((id==GUIMain::GUI_ID_ARPA_ON_BOX || id==GUIMain::GUI_ID_BIG_ARPA_ON_BOX) && event.GUIEvent.EventType == gui::EGET_CHECKBOX_CHANGED) {
                //ARPA on/off checkbox
                bool boxState = ((gui::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
                model->setArpaOn(boxState);

                //Set both linked inputs - brute force
                gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_ON_BOX,true);
                if(other!=0) {
                    ((gui::IGUICheckBox*)other)->setChecked(boxState);
                }
                other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_ON_BOX,true);
                if(other!=0) {
                    ((gui::IGUICheckBox*)other)->setChecked(boxState);
                }

            }

            if ( (id==GUIMain::GUI_ID_ARPA_VECTOR_TIME_BOX || id==GUIMain::GUI_ID_BIG_ARPA_VECTOR_TIME_BOX) && (event.GUIEvent.EventType == gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == gui::EGET_ELEMENT_FOCUS_LOST ) ) {
                std::wstring boxWString = std::wstring(((gui::IGUIEditBox*)event.GUIEvent.Caller)->getText());
                std::string boxString(boxWString.begin(), boxWString.end());
                f32 value = Utilities::lexical_cast<f32>(boxString);

                if (value > 0 && value <= 60) {
                    model->setRadarARPAVectors(value);
                } else {
                    event.GUIEvent.Caller->setText(L"Invalid");
                }

                //Set both linked inputs - brute force
                gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_VECTOR_TIME_BOX,true);
                if(other!=0) {
                    other->setText(event.GUIEvent.Caller->getText());
                }
                other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_VECTOR_TIME_BOX,true);
                if(other!=0) {
                    other->setText(event.GUIEvent.Caller->getText());
                }
            }

            //Radar PI controls
            //PI selected
            if (event.GUIEvent.EventType == gui::EGET_COMBO_BOX_CHANGED) {
                if (id == GUIMain::GUI_ID_PI_SELECT_BOX || id == GUIMain::GUI_ID_BIG_PI_SELECT_BOX) {

                    //Set to match
                    if (id == GUIMain::GUI_ID_PI_SELECT_BOX) { //Selected on small screen
                        gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_SELECT_BOX,true);
                        if(other!=0) {
                            ((gui::IGUIComboBox*)other)->setSelected(((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                        }
                    } else { //Selected on big screen
                        gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_SELECT_BOX,true);
                        if(other!=0) {
                            ((gui::IGUIComboBox*)other)->setSelected(((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected());
                        }
                    }

                    //Get PI data for the newly selected PI
                    s32 selectedPI = ((gui::IGUIComboBox*)event.GUIEvent.Caller)->getSelected(); //(-1 or 0-9)
                    //TODO: Use this to get data from model, and set fields
                    gui::IGUIElement* piBrg = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX,true);
                    gui::IGUIElement* piRng = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX,true);
                    gui::IGUIElement* piBrgBig = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_BEARING_BOX,true);
                    gui::IGUIElement* piRngBig = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_RANGE_BOX,true);
                    if (piBrg && piRng && piBrgBig && piRngBig) {
                        piBrg->setText(f32To3dp(model->getPIbearing(selectedPI),true).c_str());
                        piBrgBig->setText(f32To3dp(model->getPIbearing(selectedPI),true).c_str());
                        piRng->setText(f32To3dp(model->getPIrange(selectedPI),true).c_str());
                        piRngBig->setText(f32To3dp(model->getPIrange(selectedPI),true).c_str());
                    }
                }

            }

            if (event.GUIEvent.EventType == gui::EGET_EDITBOX_CHANGED || event.GUIEvent.EventType == gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == gui::EGET_ELEMENT_FOCUS_LOST ) {

                //Bearing/range boxes:
                if (id == GUIMain::GUI_ID_PI_BEARING_BOX || id == GUIMain::GUI_ID_BIG_PI_BEARING_BOX || id == GUIMain::GUI_ID_PI_RANGE_BOX || id == GUIMain::GUI_ID_BIG_PI_RANGE_BOX ) {
                    //Make the controls match
                    if (id == GUIMain::GUI_ID_PI_BEARING_BOX) {
                        gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_BEARING_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }
                    if (id == GUIMain::GUI_ID_BIG_PI_BEARING_BOX) {
                        gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }
                    if (id == GUIMain::GUI_ID_PI_RANGE_BOX) {
                        gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_PI_RANGE_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }
                    if (id == GUIMain::GUI_ID_BIG_PI_RANGE_BOX) {
                        gui::IGUIElement* other = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX,true);
                        if (other!=0) {other->setText(event.GUIEvent.Caller->getText());}
                    }

                    if (event.GUIEvent.EventType == gui::EGET_EDITBOX_ENTER || event.GUIEvent.EventType == gui::EGET_ELEMENT_FOCUS_LOST ) {
                        //Use the result
                        gui::IGUIElement* piCombo = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_SELECT_BOX,true);
                        gui::IGUIElement* piBrg = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_BEARING_BOX,true);
                        gui::IGUIElement* piRng = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_PI_RANGE_BOX,true);
                        if (piCombo && piBrg && piRng) {
                            s32 selectedPI = ((gui::IGUIComboBox*)piCombo)->getSelected(); //(0-9)

                            std::wstring brgWString = std::wstring(piBrg->getText());
                            std::string brgString(brgWString.begin(), brgWString.end());
                            f32 bearingChosen = Utilities::lexical_cast<f32>(brgString);

                            std::wstring rngWString = std::wstring(piRng->getText());
                            std::string rngString(rngWString.begin(), rngWString.end());
                            f32 rangeChosen = Utilities::lexical_cast<f32>(rngString);

                            //Apply to model
                            model->setPIData(selectedPI,bearingChosen,rangeChosen);

                        }
                    }

                }

            }


        } //GUI Event


        //From keyboard
        if (event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown) {
            //Check here that there isn't focus on a GUI edit box. If we are, don't process key inputs here.
            gui::IGUIElement* focussedElement = device->getGUIEnvironment()->getFocus();
            if ( !(focussedElement && focussedElement->getType()==gui::EGUIET_EDIT_BOX)) {

                if (event.KeyInput.Shift) {
                    //Shift down

                } else if (event.KeyInput.Control) {
                    //Ctrl down

                    switch(event.KeyInput.Key)
                    {
                        //Camera look
                        case KEY_UP:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookAhead();
                            break;
                        case KEY_DOWN:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookAstern();
                            break;
                        case KEY_LEFT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookPort();
                            break;
                        case KEY_RIGHT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookStbd();
                            break;

                        case KEY_KEY_M:
                            model->retrieveManOverboard();
                            break;

                        default:
                            //don't do anything
                            break;
                    }

                } else {
                    //Shift and Ctrl not down

                    switch(event.KeyInput.Key)
                    {
                        //Accelerator
                        case KEY_KEY_0:
                            model->setAccelerator(0.0);
                            break;
                        case KEY_RETURN:
                            model->setAccelerator(1.0);
                            break;
                        case KEY_KEY_1:
                            model->setAccelerator(1.0);
                            break;
                        case KEY_KEY_2:
                            model->setAccelerator(2.0);
                            break;
                        case KEY_KEY_3:
                            model->setAccelerator(5.0);
                            break;
                        case KEY_KEY_4:
                            model->setAccelerator(15.0);
                            break;
                        case KEY_KEY_5:
                            model->setAccelerator(30.0);
                            break;
                        case KEY_KEY_6:
                            model->setAccelerator(60.0);
                            break;
                        case KEY_KEY_7:
                            model->setAccelerator(3600.0);
                            break;

						case KEY_KEY_H:
							model->startHorn();
							break;

                        //Camera look
                        case KEY_UP:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookUp();
                            break;
                        case KEY_DOWN:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookDown();
                            break;
                        case KEY_LEFT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookLeft();
                            break;
                        case KEY_RIGHT:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->lookRight();
                            break;
                        case KEY_SPACE:
                            device->getGUIEnvironment()->setFocus(0); //Remove focus if space key is pressed, otherwise we get weird effects when the user changes view (as space bar toggles focussed GUI element)
                            model->changeView();
                            break;

                        //toggle full screen 3d
                        case KEY_KEY_F:
                            gui->toggleShow2dInterface();
                            break;

                        //Quit with esc or F4 (for alt-F4)
                        case KEY_ESCAPE:
                        case KEY_F4:
                            model->setAccelerator(0.0);
                            device->sleep(500);
                            device->clearSystemMessages();
                            device->getGUIEnvironment()->addMessageBox(L"Quit?",L"Quit?",true,gui::EMBF_OK|gui::EMBF_CANCEL,0,GUIMain::GUI_ID_CLOSE_BOX);//I18n
                            break;

                        case KEY_KEY_M:
                            model->releaseManOverboard();
                            break;

                        //Keyboard control of engines
                        case KEY_KEY_A:
                            //Increase port engine revs:
                            model->setPortEngine(model->getPortEngine()+0.1); //setPortEngine clamps the setting to the allowable range
                            break;
                        case KEY_KEY_Z:
                            //Decrease port engine revs:
                            model->setPortEngine(model->getPortEngine()-0.1); //setPortEngine clamps the setting to the allowable range
                            break;
                        case KEY_KEY_S:
                            //Increase stbd engine revs:
                            model->setStbdEngine(model->getStbdEngine()+0.1); //setPortEngine clamps the setting to the allowable range
                            break;
                        case KEY_KEY_X:
                            //Decrease stbd engine revs:
                            model->setStbdEngine(model->getStbdEngine()-0.1); //setPortEngine clamps the setting to the allowable range
                            break;
                        case KEY_KEY_D:
                            //Increase stbd and port engine revs:
                            model->setStbdEngine(model->getStbdEngine()+0.1); //setPortEngine clamps the setting to the allowable range
                            model->setPortEngine(model->getPortEngine()+0.1); //setPortEngine clamps the setting to the allowable range
                            break;
                        case KEY_KEY_C:
                            //Decrease stbd engine revs:
                            model->setStbdEngine(model->getStbdEngine()-0.1); //setPortEngine clamps the setting to the allowable range
                            model->setPortEngine(model->getPortEngine()-0.1); //setPortEngine clamps the setting to the allowable range
                            break;

// DEE vvvv key rudder to port changed to rudder wheel to port
                        case KEY_KEY_V:
//                            model->setRudder(model->getRudder()-5);
			    model->setWheel(model->getWheel()-1);
                            break;
// DEE ^^^^

// DEE vvvV key rudder to starboard changed to key wheel to starboard
                        case KEY_KEY_B:
//                            model->setRudder(model->getRudder()+5);
                            model->setWheel(model->getWheel()+1);
                            break;
// DEE ^^^^

                        default:
                            //don't do anything
                            break;
                    }
                }
            }
		}

		if (event.EventType == EET_KEY_INPUT_EVENT && !event.KeyInput.PressedDown) {
			if (event.KeyInput.Key == KEY_KEY_H) {
				model->endHorn();
			}
		}

		//From joystick (actually polled, once per run():
        if (event.EventType == EET_JOYSTICK_INPUT_EVENT) {


            irr::f32 newJoystickPort = previousJoystickPort;
            irr::f32 newJoystickStbd = previousJoystickStbd;
            irr::f32 newJoystickRudder = previousJoystickRudder;
            irr::f32 newJoystickBowThruster = previousJoystickBowThruster;
            irr::f32 newJoystickSternThruster = previousJoystickSternThruster;

            u8 thisJoystick = event.JoystickEvent.Joystick;
            for (u8 thisAxis = 0; thisAxis < event.JoystickEvent.NUMBER_OF_AXES; thisAxis++) {

                //Check which type we correspond to
                if (thisJoystick == joystickSetup.portJoystickNo && thisAxis == joystickSetup.portJoystickAxis) {
                    newJoystickPort = event.JoystickEvent.Axis[joystickSetup.portJoystickAxis]/32768.0;
                    //If previous value is NAN, store current value in previous and current, otherwise only in current
                    if (previousJoystickPort==INFINITY) {
                        previousJoystickPort = newJoystickPort;
                    }
                }
                if (thisJoystick == joystickSetup.stbdJoystickNo && thisAxis == joystickSetup.stbdJoystickAxis) {
                    newJoystickStbd = event.JoystickEvent.Axis[joystickSetup.stbdJoystickAxis]/32768.0;
                    //If previous value is NAN, store current value in previous and current, otherwise only in current
                    if (previousJoystickStbd==INFINITY) {
                        previousJoystickStbd = newJoystickStbd;
                    }
                }
                if (thisJoystick == joystickSetup.rudderJoystickNo && thisAxis == joystickSetup.rudderJoystickAxis) {
                    newJoystickRudder = 30*event.JoystickEvent.Axis[joystickSetup.rudderJoystickAxis]/32768.0;
                    //If previous value is NAN, store current value in previous and current, otherwise only in current
                    if (previousJoystickRudder==INFINITY) {
                        previousJoystickRudder = newJoystickRudder;
                    }
                }

                if (thisJoystick == joystickSetup.bowThrusterJoystickNo && thisAxis == joystickSetup.bowThrusterJoystickAxis) {
                    newJoystickBowThruster = event.JoystickEvent.Axis[joystickSetup.bowThrusterJoystickAxis]/32768.0;
                    //If previous value is NAN, store current value in previous and current, otherwise only in current
                    if (previousJoystickBowThruster==INFINITY) {
                        previousJoystickBowThruster = newJoystickBowThruster;
                    }
                }
                if (thisJoystick == joystickSetup.sternThrusterJoystickNo && thisAxis == joystickSetup.sternThrusterJoystickAxis) {
                    newJoystickSternThruster = event.JoystickEvent.Axis[joystickSetup.sternThrusterJoystickAxis]/32768.0;
                    //If previous value is NAN, store current value in previous and current, otherwise only in current
                    if (previousJoystickSternThruster==INFINITY) {
                        previousJoystickSternThruster = newJoystickSternThruster;
                    }
                }

            }

            //Do joystick stuff here

            //check if any have changed
            bool joystickChanged = false;
            f32 portChange = fabs(newJoystickPort - previousJoystickPort);
            f32 stbdChange = fabs(newJoystickStbd - previousJoystickStbd);
            f32 wheelChange = fabs(newJoystickRudder - previousJoystickRudder);

// DEE
//            f32 rudderChange = fabs(newJoystickRudder - previousJoystickRudder);
            f32 bowThrusterChange = fabs(newJoystickBowThruster - previousJoystickBowThruster);
            f32 sternThrusterChange = fabs(newJoystickSternThruster - previousJoystickSternThruster);
// DEE
//            if (portChange > 0.01 || stbdChange > 0.01 || rudderChange > 0.01 || bowThrusterChange > 0.01 || sternThrusterChange > 0.01 )
            if (portChange > 0.01 || stbdChange > 0.01 || wheelChange > 0.01 || bowThrusterChange > 0.01 || sternThrusterChange > 0.01 )
            {
                joystickChanged = true;
            }

            //If any have changed, use all (iff non-infinite)
            if (joystickChanged) {
                if (newJoystickPort<INFINITY) {
                    irr::f32 mappedValue = lookup1D(newJoystickPort,joystickSetup.inputPoints, joystickSetup.outputPoints);
                    model->setPortEngine(mappedValue);
                    previousJoystickPort=newJoystickPort;
                }

                if (newJoystickStbd<INFINITY) {
                    irr::f32 mappedValue = lookup1D(newJoystickStbd,joystickSetup.inputPoints, joystickSetup.outputPoints);
                    model->setStbdEngine(mappedValue);
                    previousJoystickStbd=newJoystickStbd;
                }

                if (newJoystickRudder<INFINITY) {

// DEE if the joystick rudder control is used then make it change the wheel not the rudder
                    model->setWheel(newJoystickRudder);
//                    model->setRudder(newJoystickRudder);
                    previousJoystickRudder=newJoystickRudder;
                }

                if (newJoystickBowThruster<INFINITY) {
                    model->setBowThruster(newJoystickBowThruster);
                    previousJoystickBowThruster=newJoystickBowThruster;
                }

                if (newJoystickSternThruster<INFINITY) {
                    model->setSternThruster(newJoystickSternThruster);
                    previousJoystickSternThruster=newJoystickSternThruster;
                }

            }


        }

        return false;

    }

    irr::f32 MyEventReceiver::lookup1D(irr::f32 lookupValue, std::vector<irr::f32> inputPoints, std::vector<irr::f32> outputPoints)
    {
        //Check that the input and output points list are the same length
        if (inputPoints.size() != outputPoints.size() || inputPoints.size() < 2) {
            std::cout << "Error: lookup1D needs inputPoints and outputPoints list size to be the same, and needs at least two points." << std::endl;
            exit(EXIT_FAILURE);
        }

        std::vector<f32>::size_type numberOfPoints = inputPoints.size();

        //Check that inputPoints does not have decreasing values (must be increasing or equal)
        for (unsigned int i=0; i+1<numberOfPoints; i++) {
            if (inputPoints.at(i+1) < inputPoints.at(i)) {
                std::cout << "Error: inputPoints to lookup1D must not be in a decreasing order." << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        //Return first output if at or below lowest input
        if (lookupValue <= inputPoints.at(0)) {
            return outputPoints.at(0);
        }

        //Return last output if at or above highest input
        if (lookupValue >= inputPoints.at(numberOfPoints-1)) {
            return outputPoints.at(numberOfPoints-1);
        }

        //Main interpolation
        //Find the first point above the one we're interested in
        unsigned int nextPoint=1;
        while (nextPoint < numberOfPoints && inputPoints.at(nextPoint)<=lookupValue) {
            nextPoint++;
        }

        //check for div by zero - shouldn't happen, but protect against
        if (inputPoints.at(nextPoint)-inputPoints.at(nextPoint-1) == 0)
            return 0.0;

        //do interpolation
        return outputPoints.at(nextPoint-1) + (outputPoints.at(nextPoint)-outputPoints.at(nextPoint-1))*(lookupValue-inputPoints.at(nextPoint-1))/(inputPoints.at(nextPoint)-inputPoints.at(nextPoint-1));


    }

    std::wstring MyEventReceiver::f32To3dp(irr::f32 value, bool stripZeros)
    {
        //Convert a floating point value to a wstring, with 3dp
        char tempStr[100];
        snprintf(tempStr,100,"%.3f",value);
        std::wstring outputWstring = std::wstring(tempStr, tempStr+strlen(tempStr));
        //Strip trailing zeros and decimal point
        if (stripZeros) {
            while (outputWstring.back() == '0') {
                outputWstring.pop_back();
            }
            if (outputWstring.back() == '.') {
                outputWstring.pop_back();
            }
        }
        return outputWstring;
    }

/*
	s32 MyEventReceiver::GetScrollBarPosSpeed() const
	{
		return scrollBarPosSpeed;
	}

	s32 MyEventReceiver::GetScrollBarPosHeading() const
	{
		return scrollBarPosHeading;
	}
*/
