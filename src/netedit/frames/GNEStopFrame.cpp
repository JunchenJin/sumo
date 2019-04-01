/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEStopFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    March 2019
/// @version $Id$
///
// The Widget for add Stops elements
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <netbuild/NBNetBuilder.h>
#include <netedit/GNEApplicationWindow.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/additionals/GNEAdditional.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/demandelements/GNEDemandElement.h>
#include <netedit/demandelements/GNERouteHandler.h>
#include <netedit/netelements/GNEConnection.h>
#include <netedit/netelements/GNEEdge.h>
#include <netedit/netelements/GNELane.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/xml/SUMOSAXAttributesImpl_Cached.h>

#include "GNEStopFrame.h"

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEStopFrame::StopParentSelector) StopParentSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_TYPE,    GNEStopFrame::StopParentSelector::onCmdSelectStopParent),
};

// Object implementation
FXIMPLEMENT(GNEStopFrame::StopParentSelector,     FXGroupBox, StopParentSelectorMap,       ARRAYNUMBER(StopParentSelectorMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEStopFrame::StopParentSelector - methods
// ---------------------------------------------------------------------------

GNEStopFrame::StopParentSelector::StopParentSelector(GNEStopFrame* StopFrameParent) :
    FXGroupBox(StopFrameParent->myContentFrame, "Stop parent", GUIDesignGroupBoxFrame),
    myStopFrameParent(StopFrameParent),
    myCurrentStopParent(nullptr) {
    // Create FXComboBox
    myStopParentMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    // StopParentSelector is always shown
    show();
}


GNEStopFrame::StopParentSelector::~StopParentSelector() {}


GNEDemandElement*
GNEStopFrame::StopParentSelector::getCurrentStopParent() const {
    return myCurrentStopParent;
}


void
GNEStopFrame::StopParentSelector::setStopParent(GNEDemandElement *stopParent) {
    myCurrentStopParent = stopParent;
}


void 
GNEStopFrame::StopParentSelector::showStopParentSelector() {
    // refresh stop parent selector
    refreshStopParentSelector();
    // show VType selector
    show();
}


void 
GNEStopFrame::StopParentSelector::hideStopParentSelector() {
    hide();
}


void 
GNEStopFrame::StopParentSelector::refreshStopParentSelector() {
    // clear comboBox and show
    myStopParentMatchBox->clearItems();
    myStopParentMatchBox->show();
    myStopParentCandidates.clear();
    // reserve stop parent cantidadtes
    myStopParentCandidates.reserve(
        myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_ROUTE).size() +
        myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_VEHICLE).size() +
        myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_FLOW).size() +
        myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_TRIP).size());
    // fill myStopParentMatchBox with list of routes
    for (const auto& i : myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_ROUTE)) {
        myStopParentMatchBox->appendItem(i.first.c_str());
        myStopParentCandidates.push_back(i.second);
    }
    // fill myStopParentMatchBox with list of vehicles
    for (const auto& i : myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_VEHICLE)) {
        myStopParentMatchBox->appendItem(i.first.c_str());
        myStopParentCandidates.push_back(i.second);
    }
    // fill myStopParentMatchBox with list of flows
    for (const auto& i : myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_FLOW)) {
        myStopParentMatchBox->appendItem(i.first.c_str());
        myStopParentCandidates.push_back(i.second);
    }
    // fill myStopParentMatchBox with list of trips
    for (const auto& i : myStopFrameParent->getViewNet()->getNet()->getDemandElementByType(SUMO_TAG_TRIP)) {
        myStopParentMatchBox->appendItem(i.first.c_str());
        myStopParentCandidates.push_back(i.second);
    }
    // Set visible items
    if ((int)myStopParentMatchBox->getNumItems() < 20) {
        myStopParentMatchBox->setNumVisible((int)myStopParentMatchBox->getNumItems());
    } else {
        myStopParentMatchBox->setNumVisible(20);
    }
    // check if myCurrentStopParent exists
    bool found = false;
    for (int i = 0; i < (int)myStopParentCandidates.size(); i++) {
        if (myStopParentCandidates.at(i) == myCurrentStopParent) {
            myStopParentMatchBox->setCurrentItem(i);
            found = true;
        }
    }
    if (!found) {
        // disable combo box and moduls if there isn't candidate stop parents in net
        if (myStopParentCandidates.size() > 0) {
            myStopParentMatchBox->setCurrentItem(0);
        } else {
            myStopParentMatchBox->hide();
            myStopFrameParent->myStopTypeSelector->hideItemSelector();
            myStopFrameParent->myStopAttributes->hideAttributesCreatorModul();
            myStopFrameParent->myNeteditAttributes->hideNeteditAttributesModul();
            myStopFrameParent->myHelpCreation->hideHelpCreation();
        }
    }
}


long
GNEStopFrame::StopParentSelector::onCmdSelectStopParent(FXObject*, FXSelector, void*) {
    // Check if value of myStopParentMatchBox correspond to a existent stop parent candidate
    for (int i = 0; i < (int)myStopParentCandidates.size(); i++) {
        if (myStopParentCandidates.at(i)->getID() == myStopParentMatchBox->getText().text()) {
            // set color of myStopParentMatchBox to black (valid)
            myStopParentMatchBox->setTextColor(FXRGB(0, 0, 0));
            // Set new current VType
            myCurrentStopParent = myStopParentCandidates.at(i);
            // show Stop selector, attributes and help creation moduls
            myStopFrameParent->myStopTypeSelector->showItemSelector();
            myStopFrameParent->myStopAttributes->showAttributesCreatorModul(myStopFrameParent->myStopTypeSelector->getCurrentTagProperties());
            myStopFrameParent->myNeteditAttributes->showNeteditAttributesModul(myStopFrameParent->myStopTypeSelector->getCurrentTagProperties());
            myStopFrameParent->myHelpCreation->showHelpCreation();
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myStopParentMatchBox->getText() + "' in StopParentSelector").text());
            return 1;
        }
    }
    // if VType selecte is invalid, select
    myCurrentStopParent = nullptr;
    // hide all moduls if selected item isn't valid
    myStopFrameParent->myStopTypeSelector->hideItemSelector();
    myStopFrameParent->myStopAttributes->hideAttributesCreatorModul();
    myStopFrameParent->myNeteditAttributes->hideNeteditAttributesModul();
    myStopFrameParent->myHelpCreation->hideHelpCreation();
    // set color of myStopParentMatchBox to red (invalid)
    myStopParentMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in StopParentSelector");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEStopFrame::HelpCreation - methods
// ---------------------------------------------------------------------------

GNEStopFrame::HelpCreation::HelpCreation(GNEStopFrame* StopFrameParent) :
    FXGroupBox(StopFrameParent->myContentFrame, "Help", GUIDesignGroupBoxFrame),
    myStopFrameParent(StopFrameParent) {
    myInformationLabel = new FXLabel(this, "", 0, GUIDesignLabelFrameInformation);
}


GNEStopFrame::HelpCreation::~HelpCreation() {}


void 
GNEStopFrame::HelpCreation::showHelpCreation() {
    // first update help cration
    updateHelpCreation();
    // show modul
    show();
}


void 
GNEStopFrame::HelpCreation::hideHelpCreation() {
    hide();
}


void 
GNEStopFrame::HelpCreation::updateHelpCreation() {
    // create information label
    std::ostringstream information;
    // set text depending of selected Stop type
    switch (myStopFrameParent->myStopTypeSelector->getCurrentTagProperties().getTag()) {
        case SUMO_TAG_STOP_BUSSTOP:
            information
                << "- Click over a bus stop\n"
                << "  to create a stop.";
            break;
        case SUMO_TAG_STOP_CONTAINERSTOP:
            information
                << "- Click over a container stop\n"
                << "  to create a stop.";
            break;
        case SUMO_TAG_STOP_CHARGINGSTATION:
            information
                << "- Click over a charging \n"
                << "  station to create a stop.";
            break;
        case SUMO_TAG_STOP_PARKINGAREA:
            information
                << "- Click over a parking area\n"
                << "  to create a stop.";
            break;
        case SUMO_TAG_STOP_LANE:
            information
                << "- Click over a lane to\n"
                << "  create a stop.";
            break;
        default:
            break;
    }
    // set information label
    myInformationLabel->setText(information.str().c_str());
}

// ---------------------------------------------------------------------------
// GNEStopFrame - methods
// ---------------------------------------------------------------------------

GNEStopFrame::GNEStopFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Stops") {

    // Create Stop parent selector
    myStopParentSelector = new StopParentSelector(this);

    // Create item Selector modul for Stops
    myStopTypeSelector = new ItemSelector(this, GNEAttributeCarrier::TagType::TAGTYPE_STOP);

    // Create Stop parameters
    myStopAttributes = new AttributesCreator(this);
    
    // Create Netedit parameter
    myNeteditAttributes = new NeteditAttributes(this);

    // Create Help Creation Modul
    myHelpCreation = new HelpCreation(this);

    // refresh myStopParentMatchBox
    myStopParentSelector->refreshStopParentSelector();
}


GNEStopFrame::~GNEStopFrame() {}


void
GNEStopFrame::show() {
    // refresh vType selector
    myStopParentSelector->refreshStopParentSelector();
    // refresh item selector
    myStopTypeSelector->refreshTagProperties();
    // show frame
    GNEFrame::show();
}


bool
GNEStopFrame::addStop(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // now check if stop parent selector is valid
    if (myStopParentSelector->getCurrentStopParent() == nullptr) {
        WRITE_WARNING("Current selected Stop parent isn't valid.");
        return false;
    }

    /*
    const int STOP_INDEX_END = -1;
    const int STOP_INDEX_FIT = -2;

    const int STOP_END_SET = 1;
    const int STOP_START_SET = 2;
    const int STOP_TRIGGER_SET = 2 << 1;
    const int STOP_PARKING_SET = 2 << 2;
    const int STOP_EXPECTED_SET = 2 << 3;
    const int STOP_CONTAINER_TRIGGER_SET = 2 << 4;
    const int STOP_EXPECTED_CONTAINERS_SET = 2 << 5;
    const int STOP_TRIP_ID_SET = 2 << 6;
    */

    // obtain tag (only for improve code legibility)
    SumoXMLTag stopTag = myStopTypeSelector->getCurrentTagProperties().getTag();

    // declare a Stop
    SUMOVehicleParameter::Stop stopParameter;

    // first check that current selected Stop is valid
    if (stopTag == SUMO_TAG_NOTHING) {
        WRITE_WARNING("Current selected Stop type isn't valid.");
        return false;
    } else if (stopTag == SUMO_TAG_STOP_LANE) {
        if(objectsUnderCursor.getLaneFront()) {
            stopParameter.lane = objectsUnderCursor.getLaneFront()->getID();
             stopParameter.parametersSet;
        } else {
            WRITE_WARNING("Click over a lane to create a stop placed in a lane");
            return false;
        }
    } else if (objectsUnderCursor.getAdditionalFront()) {
        if (objectsUnderCursor.getAdditionalFront()->getTagProperty().getTag() == SUMO_TAG_BUS_STOP) {
            if (stopTag != SUMO_TAG_STOP_BUSSTOP) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a busstop");
                return false;
            } else {
                stopParameter.busstop = objectsUnderCursor.getAdditionalFront()->getID();
                stopParameter.startPos = 0;
                stopParameter.endPos = 0;
            }
        } else if (objectsUnderCursor.getAdditionalFront()->getTagProperty().getTag() == SUMO_TAG_CONTAINER_STOP) {
            if (stopTag != SUMO_TAG_STOP_CONTAINERSTOP) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a containerStop");
                return false;
            } else {
                stopParameter.containerstop = objectsUnderCursor.getAdditionalFront()->getID();
                stopParameter.startPos = 0;
                stopParameter.endPos = 0;
            }
        } else if (objectsUnderCursor.getAdditionalFront()->getTagProperty().getTag() == SUMO_TAG_CHARGING_STATION) {
            if (stopTag != SUMO_TAG_STOP_CHARGINGSTATION) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a chargingStation");
                return false;
            } else {
                stopParameter.containerstop = objectsUnderCursor.getAdditionalFront()->getID();
                stopParameter.startPos = 0;
                stopParameter.endPos = 0;
            }
        } else if (objectsUnderCursor.getAdditionalFront()->getTagProperty().getTag() == SUMO_TAG_PARKING_AREA) {
            if (stopTag != SUMO_TAG_STOP_PARKINGAREA) {
                WRITE_WARNING("Invalid clicked stopping place to create a stop placed in a parkingArea");
                return false;
            } else {
                stopParameter.parkingarea = objectsUnderCursor.getAdditionalFront()->getID();
                stopParameter.startPos = 0;
                stopParameter.endPos = 0;
            }
        }
    } else {
        if (stopTag == SUMO_TAG_STOP_BUSSTOP) {
            WRITE_WARNING("Click over a busStop to create a stop placed in a busstop");
            return false;
        } else if (stopTag != SUMO_TAG_STOP_CONTAINERSTOP) {
            WRITE_WARNING("Click over a containerStop to create a stop placed in a containerStop");
            return false;
        } else if (stopTag != SUMO_TAG_CHARGING_STATION) {
            WRITE_WARNING("Click over a chargingStation to create a stop placed in a chargingStation");
            return false;
        } else if (stopTag != SUMO_TAG_STOP_PARKINGAREA) {
            WRITE_WARNING("Click over a parkingArea to create a stop placed in a parkingArea");
            return false;
        }
    }

    // declare map to keep attributes from Frames from Frame
    std::map<SumoXMLAttr, std::string> valuesMap = myStopAttributes->getAttributesAndValues(true);

    // generate ID
    valuesMap[SUMO_ATTR_ID] = myViewNet->getNet()->generateDemandElementID(stopTag);

    // add netedit values
    if (!stopParameter.lane.empty()) {
        myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, objectsUnderCursor.getLaneFront());
        stopParameter.startPos = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_STARTPOS]);
        stopParameter.endPos = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_ENDPOS]);
    }

    // fill rest of parameters
    stopParameter.duration = string2time(valuesMap[SUMO_ATTR_DURATION]);
    stopParameter.until = string2time(valuesMap[SUMO_ATTR_UNTIL]);
    stopParameter.triggered = GNEAttributeCarrier::parse<bool>(valuesMap[SUMO_ATTR_TRIGGERED]);
    stopParameter.containerTriggered = GNEAttributeCarrier::parse<bool>(valuesMap[SUMO_ATTR_CONTAINER_TRIGGERED]);
    stopParameter.parking = GNEAttributeCarrier::parse<bool>(valuesMap[SUMO_ATTR_PARKING]);
    stopParameter.awaitedPersons = GNEAttributeCarrier::parse<std::set<std::string> >(valuesMap[SUMO_ATTR_EXPECTED]);
    stopParameter.awaitedContainers = GNEAttributeCarrier::parse<std::set<std::string> >(valuesMap[SUMO_ATTR_EXPECTED_CONTAINERS]);
    /*
    stopParameter.tripId = valuesMap[SUMO_ATTR_ENDPOS];
    stopParameter.accessPos = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_ENDPOS]);
    stopParameter.index = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_ENDPOS]);
    */
   
    // create it in RouteFrame
    GNERouteHandler::buildStop(myViewNet, true, stopParameter, myStopParentSelector->getCurrentStopParent());

    // nothing crated
    return false;
}


// ===========================================================================
// protected
// ===========================================================================

void
GNEStopFrame::enableModuls(const GNEAttributeCarrier::TagProperties& tagProperties) {
    // show Stop type selector modul
    myStopAttributes->showAttributesCreatorModul(tagProperties);
    myNeteditAttributes->showNeteditAttributesModul(tagProperties);
    myHelpCreation->showHelpCreation();
}


void
GNEStopFrame::disableModuls() {
    // hide all moduls if Stop isn't valid
    myStopAttributes->hideAttributesCreatorModul();
    myNeteditAttributes->hideNeteditAttributesModul();
    myHelpCreation->hideHelpCreation();
}

/****************************************************************************/
