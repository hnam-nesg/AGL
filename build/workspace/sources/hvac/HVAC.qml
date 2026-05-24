// /*
//  * Copyright (C) 2016 The Qt Company Ltd.
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *      http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */

// import QtQuick 2.6
// import QtQuick.Layouts 1.1
// import QtQuick.Controls 2.0
// import QtQuick.Window 2.11
// import AGL.Demo.Controls 1.0
// import Translator 1.0

// ApplicationWindow {
//     id: root

//     width: container.width * container.scale
//     height: container.height * container.scale

// 	Translator {
// 		id: translator
// 		language: "en_US"
// 	}

// 	Connections {
// 		target: hvac
// 		onFanSpeedChanged: fanSpeedSlider.value = fanSpeed
// 		onLanguageChanged: translator.language = language
// 	}

//     Item {
//         id: container
//         anchors.centerIn: parent
//         width: Window.width
//         height: Window.height

// 	ColumnLayout {
// 		anchors.fill: parent
// 		anchors.topMargin: width / 10
// 		anchors.bottomMargin: width / 10
// 		RowLayout {
// 			Layout.fillHeight: true
// 			Layout.alignment: Qt.AlignHCenter
// 			Image {
// 				source: 'qrc:/images/HMI_HVAC_Fan_Icon.svg'
// 			}
// 			Item {
// 				width: 1080 * 0.8
// 				Slider {
// 					id: fanSpeedSlider
// 					anchors.left: parent.left
// 					anchors.right: parent.right
// 					anchors.verticalCenter: parent.verticalCenter
// 					from: 0.0
// 					to: 255.0
// 					stepSize: 1.0
// 					onValueChanged: {
// 						hvac.fanSpeed = value
// 					}
// 				}
// 				Label {
// 					anchors.left: fanSpeedSlider.left
// 					anchors.top: fanSpeedSlider.bottom
// 					font.pixelSize: 32
// 					text: translator.translate(qsTr('FAN SPEED'), translator.language)
// 				}
// 			}
// 		}
// 		RowLayout {
// 			Layout.fillHeight: true
// 			Layout.fillWidth: true
// 			Layout.alignment: Layout.Center
// 			spacing: 20
// 			ColumnLayout {
// 				Layout.fillWidth: true
// 				spacing: 20
// 				SeatHeatButton {
// 					id: leftSeat
// 					side: 'Left'
// 				}
// 				HeatDegree {
// 					onCurrentItemChanged: {
// 						console.log("Left Temp changed",degree)
// 						hvac.leftTemperature = degree
// 					}
// 				}
// 			}
// 			ColumnLayout {
// 				Layout.fillWidth: true
// 				spacing: 20
// 				ToggleButton {
// 					onImage: 'qrc:/images/HMI_HVAC_Active.svg'
// 					offImage: 'qrc:/images/HMI_HVAC_Inactive.svg'
// 					Label {
// 						anchors.centerIn: parent
// 						color: parent.checked ? '#00ADDC' : '#848286'
// 						text: translator.translate(qsTr('A/C'), translator.language)
// 						font.pixelSize: parent.height / 3
// 					}
// 					onCheckedChanged: {
// 						console.debug('A/C', checked)
// 					}
// 				}
// 				ToggleButton {
// 					onImage: 'qrc:/images/HMI_HVAC_Active.svg'
// 					offImage: 'qrc:/images/HMI_HVAC_Inactive.svg'
// 					Label {
// 						anchors.centerIn: parent
// 						color: parent.checked ? '#00ADDC' : '#848286'
// 						text: translator.translate(qsTr('AUTO'), translator.language)
// 						font.pixelSize: parent.height / 3
// 					}
// 					onCheckedChanged: {
// 						console.debug('AUTO', checked)
// 					}
// 				}
// 				ToggleButton {
// 					onImage: 'qrc:/images/HMI_HVAC_Circulation_Active.svg'
// 					offImage: 'qrc:/images/HMI_HVAC_Circulation_Inactive.svg'
// 					onCheckedChanged: {
// 						console.debug('Circulation', checked)
// 					}
// 				}
// 			}

// 			ColumnLayout {
// 				Layout.fillWidth: true
// 				spacing: 20
// 				SeatHeatButton {
// 					id: rightSeat
// 					side: 'Right'
// 				}
// 				HeatDegree {
// 					onCurrentItemChanged: {
// 						console.log("Right Temp changed",degree)
// 						hvac.rightTemperature = degree
// 					}
// 				}
// 			}
// 		}

// 		RowLayout {
// 			Layout.fillHeight: true
// 			Layout.alignment: Qt.AlignHCenter
// 			spacing: root.width / 20
// 			Repeater {
// 				model: ['AirDown', 'AirUp', 'AirRight', 'Rear', 'Front']
// 				ToggleButton {
// 					onImage: 'qrc:/images/HMI_HVAC_%1_Active.svg'.arg(model.modelData)
// 					offImage: 'qrc:/images/HMI_HVAC_%1_Inactive.svg'.arg(model.modelData)
// 					onCheckedChanged: {
// 						console.debug(model.modelData, checked)
// 					}
// 				}
// 			}
// 		}
// 	}
// }
// }

/*
 * Copyright (c) 2018 TOYOTA MOTOR CORPORATION
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window 
import QtQuick.Effects
import QtQuick3D
import QtQuick3D.Helpers
import Qt5Compat.GraphicalEffects

ApplicationWindow {
    id: root

	property int level_FL: 0
    property int level_FR: 0
    property int level_RL: 0
    property int level_RR: 0
	property int level_fan: 0
    property string airdistribution: ""
    property bool airrecir: true
    property bool airauto: false
    property bool aircondition: false

    width: 1200
    height: 670//560
    color: "transparent"

    function formatDisplayValue(value) {
        var v = Number(value)
        if (isNaN(v))
            return "0"
        return String(Math.floor(Math.abs(v)))
    }

    function clampSeatLevel(value) {
        var level = Number(value)
        if (isNaN(level))
            return 0
        return Math.max(-3, Math.min(3, Math.round(level)))
    }

    function clampFanLevel(value) {
        var level = Number(value)
        if (isNaN(level))
            return 0
        return Math.max(0, Math.min(6, Math.round(level)))
    }

    function showPage(pageIndex) {
        var index = Number(pageIndex)
        if (isNaN(index))
            return

        index = Math.max(0, Math.min(swipe.count - 1, Math.round(index)))
        swipe.currentIndex = index
    }

    function updateSeatLevel(path, value) {
        var level = clampSeatLevel(value)

        if (path === "Vehicle.Cabin.Seat.Row1.DriverSide.HeatingCooling") {
            root.level_FL = level
            slide_FL.value = level
        } else if (path === "Vehicle.Cabin.Seat.Row1.PassengerSide.HeatingCooling") {
            root.level_FR = level
            slide_FR.value = level
        } else if (path === "Vehicle.Cabin.Seat.Row2.DriverSide.HeatingCooling") {
            root.level_RL = level
            slide_RL.value = level
        } else if (path === "Vehicle.Cabin.Seat.Row2.PassengerSide.HeatingCooling") {
            root.level_RR = level
            slide_RR.value = level
        }
    }

    Connections {
        target: VehicleSignals

        function onConnected() {
            VehicleSignals.authorize()
        }

        function onAuthorized() {
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Driver.FanSpeed", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Driver.Temperature", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Passenger.Temperature", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsAirConditioningActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsRearDefrosterActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsFrontDefrosterActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Driver.AirDistribution", false)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsRecirculationActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsAutoPowerOptimize", true)
            VehicleSignals.subscribe("Vehicle.Cabin.Seat.Row1.DriverSide.HeatingCooling", true)
            VehicleSignals.subscribe("Vehicle.Cabin.Seat.Row1.PassengerSide.HeatingCooling", true)
            VehicleSignals.subscribe("Vehicle.Cabin.Seat.Row2.DriverSide.HeatingCooling", true)
            VehicleSignals.subscribe("Vehicle.Cabin.Seat.Row2.PassengerSide.HeatingCooling", true)
        }


        function onSignalNotification(path, value, timestamp) {
            root.updateSeatLevel(path, value)

            if (path === "Vehicle.Cabin.HVAC.Station.Row1.Driver.FanSpeed") {
                var fanLevel = root.clampFanLevel(value)
                root.level_fan = fanLevel
                fanSlider.value = fanLevel
                windflow.opacity = fanLevel / 8
            }
            if (path === "Vehicle.Cabin.HVAC.Station.Row1.Driver.AirDistribution") {
                root.airdistribution = value
                console.log("AirDistribution: ", root.airdistribution)
            }
            if (path === "Vehicle.Cabin.HVAC.IsRecirculationActive") {
                var recirValue = (value === "true" || value === true)
                root.airrecir = recirValue
            }
            if (path === "Vehicle.Cabin.HVAC.IsAutoPowerOptimize") {
                var autoValue = (value === "true" || value === true)
                root.airauto = autoValue
            }
            if (path === "Vehicle.Cabin.HVAC.IsAirConditioningActive") {
                var conditionValue = (value === "true" || value === true)
                root.aircondition = conditionValue
            }
        }
    }

    Connections {
        target: hvacUiDbus

        function onPageRequested(pageIndex) {
            root.showPage(pageIndex)
        }
    }

    Component.onCompleted: {
        VehicleSignals.connect()
    }

    Background{width: 1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}

	Rectangle{

		FontLoader{
			id: fontLoader
			source: "images/Roboto-BoldCondensed.ttf"
		}

		y: 55; width: 1196; height: 560; color: "transparent"; radius: 10
		BorderColor{}
		Rectangle {
			anchors.fill: parent
			radius: 10
			color: "transparent"
			border.width: 3
			layer.enabled: true
			layer.effect: MultiEffect {
				blurEnabled: true
				blur:0.6
			}
			BorderColor{borderWidth: 1.5; c1: "white"; c2: "white"}
		}

		SwipeView {
			id: swipe
			anchors.fill: parent
			interactive: true; clip: true
			Page {
				background: null

				Image{
                	anchors.fill: parent
                	source: "images/background.png"
            	}
				SlideBar{id: slide_FL; x: 20; y: 200; onValueChanged: {root.level_FL = value; hvac.seatDriver = value} onDisableSwipeView:{swipe.interactive = true} onEnableSwipeView:{swipe.interactive = false}}

            	SlideBar{id: slide_RL; x: 20; y: 410; onValueChanged: {root.level_RL = value; hvac.seatBehindDriver = value} onDisableSwipeView:{swipe.interactive = true} onEnableSwipeView:{swipe.interactive = false}}

            	SlideBar{id: slide_FR; x: 750; y: 200; onValueChanged: {root.level_FR = value; hvac.seatPassenger = value} onDisableSwipeView:{swipe.interactive = true} onEnableSwipeView:{swipe.interactive = false}}

            	SlideBar{id: slide_RR; x: 750; y: 410; onValueChanged: {root.level_RR = value; hvac.seatBehindPassenger = value} onDisableSwipeView:{swipe.interactive = true} onEnableSwipeView:{swipe.interactive = false}}

				Image{
					x: 533; y: 246; width: 54; height: 89
					source: "images/seat_FL_" + root.level_FL +".png"
				}
				Image{
					x: 609; y: 246; width: 54; height: 89
					source: "images/seat_FR_"+ root.level_FR + ".png"
				}
				Image {
					x: 528; y: 334; width: 60; height: 99
					source: "images/seat_RL_"+ root.level_RL + ".png"
				}

				Image {
					x: 610; y: 334; width: 60; height: 99
					source: "images/seat_RR_"+ root.level_RR + ".png"
				}

				Text{
					x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
					text: "Điều chỉnh sưởi ghế"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
					MultiEffect {
						anchors.fill: parent; source: parent; shadowEnabled: true
						shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
						shadowHorizontalOffset: 5; shadowVerticalOffset: 5
					}
				}
			}

			Page {
				background: null

				Image{
					width: 1196; height: 555; source: "images/mercedes_air.png"
				}

				Text{
					id: cc; x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
					text: "Điều chỉnh điều hòa"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
					MultiEffect {
						anchors.fill: parent; source: parent; shadowEnabled: true
						shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
						shadowHorizontalOffset: 5; shadowVerticalOffset: 5
					}
				}

				BluePanel{id: panel; x: 248; y: 420; width: 700; height: 150}

                WindSlideBar{id: fanSlider; anchors.centerIn: panel; anchors.verticalCenterOffset: 0; onValueChanged: {root.level_fan = value; hvac.fanSpeed = value; windflow.opacity = value / 8} onDisableSwipeView:{swipe.interactive = true} onEnableSwipeView:{swipe.interactive = false}}

                Column{
                    x: 70; y: 120; spacing: 30
                    Rectangle{
                        width: 120; height: 100; color: "#3f6f6f6f"; radius: 15
                        BorderColor{radius: 15}
                        Rectangle {
                            anchors.fill: parent
                            scale: 1.01
                            radius: 15
                            color: "transparent"
                            border.width: 3
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                blurEnabled: true
                                blur:0.6
                            }
                            BorderColor{borderWidth: 2; radius: 15}
                        }
                        HL_Button{id: auto;width: 120; height: 100; radius: 15; visible: root.airauto}
                        ButtonControl {
                            width: 120; height: 100; source: "images/auto.png"; width_image: 50; height_image: 50; text_sub: "Tự động"; text_direct: 40
							onClicked:{hvac.airAuto = checked; if(!root.aircondition){hvac.airCondition = checked; air.checked = checked}}
						}
                    }
                    Rectangle{
                        width: 120; height: 100; color: "#3f6f6f6f"; radius: 15
                        BorderColor{radius: 15}
                        Rectangle {
                            anchors.fill: parent
                            scale: 1.01
                            radius: 15
                            color: "transparent"
                            border.width: 3
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                blurEnabled: true
                                blur:0.6
                            }
                            BorderColor{borderWidth: 2; radius: 15}
                        }
						HL_Button{id: air_condition;width: 120; height: 100; radius: 15; visible: root.aircondition}
                        ButtonControl {
                            id: air; width: 120; height: 100; source: "images/ac.png"; width_image: 50; height_image: 50; text_sub: "Điều hòa"; text_direct: 40
							onClicked:{hvac.airCondition = checked}
						}
                    }
                    Rectangle{
                        width: 120; height: 100; color: "#3f6f6f6f"; radius: 15
                        BorderColor{radius: 15}
                        Rectangle {
                            anchors.fill: parent
                            scale: 1.01
                            radius: 15
                            color: "transparent"
                            border.width: 3
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                blurEnabled: true
                                blur:0.6
                            }
                            BorderColor{borderWidth: 2; radius: 15}
                        }
						HL_Button{id: max_air; width: 120; height: 100; radius: 15; visible: false}
                        ButtonControl {
                            width: 120; height: 100; source: "images/ac.png"; width_image: 50; height_image: 50; text_sub: "Tối đa"; text_direct: 40
							onClicked:{max_air.visible = checked}
						}
                    }
                }

                Column{
                    x: 1006; y: 120; spacing: 30
                    Rectangle{
                        width: 120; height: 100; color: "#3f6f6f6f"; radius: 15
                        BorderColor{radius: 15}
                        Rectangle {
                            anchors.fill: parent
                            scale: 1.01
                            radius: 15
                            color: "transparent"
                            border.width: 3
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                blurEnabled: true
                                blur:0.6
                            }
                            BorderColor{borderWidth: 2; radius: 15}
                        }
						HL_Button{id: fresh_air;width: 120; height: 100; radius: 15; visible: !root.airrecir}
                        ButtonControl {
                            width: 120; height: 100; source: "images/fresh_air.png"; width_image: 60; height_image: 60; text_sub: "Gió ngoài"; text_direct: 40
							onClicked:{hvac.airRecir = !checked}
						}
                    }
                    Rectangle{
                        width: 120; height: 100; color: "#3f6f6f6f"; radius: 15
                        BorderColor{radius: 15}
                        Rectangle {
                            anchors.fill: parent
                            scale: 1.01
                            radius: 15
                            color: "transparent"
                            border.width: 3
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                blurEnabled: true
                                blur:0.6
                            }
                            BorderColor{borderWidth: 2; radius: 15}
                        }
						HL_Button{id: ion; width: 120; height: 100; radius: 15; visible: false}
                        ButtonControl {
                            width: 120; height: 100; source: "images/icon.png"; width_image: 50; height_image: 50; text_sub: "Ion"; text_direct: 40
							onClicked:{ion.visible = checked}
						}
                    }
                    Rectangle{
                        width: 120; height: 100; color: "#3f6f6f6f"; radius: 15
                        BorderColor{radius: 15}
                        Rectangle {
                            anchors.fill: parent
                            scale: 1.01
                            radius: 15
                            color: "transparent"
                            border.width: 3
                            layer.enabled: true
                            layer.effect: MultiEffect {
                                blurEnabled: true
                                blur:0.6
                            }
                            BorderColor{borderWidth: 2; radius: 15}
                        }
						HL_Button{id: sync;width: 120; height: 100; radius: 15; visible: false}
                        ButtonControl {
                            width: 120; height: 100; source: "images/thermometer.png"; width_image: 50; height_image: 50; text_sub: "Đồng bộ"; text_direct: 40
							onClicked:{sync.visible = checked; hvac.syncTemp = checked}
						}
                    }
                }

                WindFlowShader {
                    id: windflow
                    x: 430; y: root.airdistribution === "MIDDLE" ? 125 : (root.airdistribution === "UP" ? 69 : 181); width: 420; height: 140
                    strandCount: 8; spread: 7.5; ampMain: 12; ampDetail: 8; thickness: 15; glowScale: 0; brightness: 0.6; speed: 0.2; opacity: 0
                }

                Rectangle{
                    x: 391; y: 120; width: 50; height: 50; color: "#3f6f6f6f"; radius: 25
                    BorderColor{radius: 25}
                    Rectangle {
                        anchors.fill: parent
                        scale: 1.01
                        radius: 25
                        color: "transparent"
                        border.width: 3
                        layer.enabled: true
                        layer.effect: MultiEffect {
                            blurEnabled: true
                            blur:0.6
                        }
                        BorderColor{borderWidth: 2; radius: 25}
                    }
                    HL_Button{width: 50; height: 50; radius: 25; radius_: 25; visible: root.airdistribution === "UP"; c1: "#0092ff"; c2: "#002e51"}
                    ButtonControl{
                        id: face; width: 50; height: 50; source: "images/face_outlet.png"; width_image: 40; height_image: 40; center: true
                        onClicked:{hvac.airDistribution = "UP"}
                    }
                }

                Rectangle{
                    x: 432; y: 176; width: 50; height: 50; color: "#3f6f6f6f"; radius: 25
                    BorderColor{radius: 25}
                    Rectangle {
                        anchors.fill: parent
                        scale: 1.01
                        radius: 25
                        color: "transparent"
                        border.width: 3
                        layer.enabled: true
                        layer.effect: MultiEffect {
                            blurEnabled: true
                            blur:0.6
                        }
                        BorderColor{borderWidth: 2; radius: 25}
                    }
                    HL_Button{width: 50; height: 50; radius: 25; radius_: 25; visible: root.airdistribution === "MIDDLE"; c1: "#0092ff"; c2: "#002e51"}
                    ButtonControl{
                        id: foot_face; width: 50; height: 50; source: "images/foot_face_outlet.png"; width_image: 40; height_image: 40; center: true
                        onClicked:{hvac.airDistribution = "MIDDLE"}
                    }
                }

                Rectangle{
                    x: 391; y: 232; width: 50; height: 50; color: "#3f6f6f6f"; radius: 25
                    BorderColor{radius: 25}
                    Rectangle {
                        anchors.fill: parent
                        scale: 1.01
                        radius: 25
                        color: "transparent"
                        border.width: 3
                        layer.enabled: true
                        layer.effect: MultiEffect {
                            blurEnabled: true
                            blur:0.6
                        }
                        BorderColor{borderWidth: 2; radius: 25}
                    }
                    HL_Button{width: 50; height: 50; radius: 25; radius_: 25; visible: root.airdistribution === "DOWN"; c1: "#0092ff"; c2: "#002e51"}
                    ButtonControl{
                        id: foot; width: 50; height: 50; source: "images/foot_outlet.png"; width_image: 40; height_image: 40; center: true
                        onClicked:{hvac.airDistribution = "DOWN"}
                    }
                }
			}
		}

		PageIndicator {
			id: indicator
			y: 20
			anchors.horizontalCenter: swipe.horizontalCenter
			count: swipe.count
			currentIndex: swipe.currentIndex
			delegate: Rectangle {
				height: 8
				width: index === indicator.currentIndex ? 32 : 8
				radius: height / 2

				color: index === indicator.currentIndex ? "#00E5FF" : "#FFFFFF"
				opacity: index === indicator.currentIndex ? 1.0 : 0.25

				Behavior on width { NumberAnimation { duration: 200 } }
				Behavior on opacity { NumberAnimation { duration: 200 } }
				Rectangle {
					anchors.fill: parent
					scale: 1.1
					color: "white"
					radius: 4
					layer.enabled: true
					layer.effect: MultiEffect {
						blurEnabled: true
						blur:0.6
					}
					BorderColor{borderWidth: 10; c1: "#00E5FF"; c2: "#00E5FF"; radius: 4}
				}
			}
		}
	}
}
