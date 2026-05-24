/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2016, 2017 Mentor Graphics Development (Deutschland) GmbH
 * Copyright (c) 2018,2019 TOYOTA MOTOR CORPORATION
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
import Qt5Compat.GraphicalEffects
import QtQuick.Effects

ApplicationWindow {
    id: window
    width: 850
    height: 670
    color: "transparent"


                // MouseArea {
                //     id: loc
                //     anchors.fill: parent
                //     property string currentApp: ''
                //     onClicked: {
                //         parent.GridView.view.currentIndex = index
                //         currentApp = ApplicationModel.appid(parent.GridView.view.currentIndex)
                //         console.debug("Launcher: Starting app " + currentApp)
                //         applauncher.startApplication(currentApp)
                //         console.debug("Launcher: Started app " + currentApp)
                //     }
                // }

    Item{
        id: root
        Background{width: 1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}

        FontLoader{
            id: fontLoader
            source: "images/Roboto-BoldCondensed.ttf"
        }

        y: 5; width: 860; height: 660
        BorderColor{}
        Rectangle {
            anchors.fill: parent
            radius: 10
            color: "#3f6f6f6f"
            border.width: 3
            layer.enabled: true
            layer.effect: MultiEffect {
                blurEnabled: true
                blur:0.6
            }
            BorderColor{borderWidth: 1; c1: "white"; c2: "white"}
        }

        Text{
            id: text_connected
            x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"; visible: true
            text: "Thư viện ứng dụng"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }

        Item {
            id: device
            x: 30; y: 80; width: 800; height: 600; clip: true

                ButtonControl{
                    id: media;width: 50; height: 50; source: "images/arrow-" + (checked ? "down" : "top") +".png"; width_image: 24; height_image: 24
                }
                Text{
                    x: 50; anchors.verticalCenter: media.verticalCenter; font.family: fontLoader.name; font.pixelSize: 25; color: "white"
                    text: "Đa phương tiện"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignLeft
                }

                Rectangle{
                    id: rect1;x: 50; y: 50; width: parent.width; height: media.checked ? 0 : 150; color: "transparent"
                    ButtonControl{
                        width: 150; height: media.checked ? 0 : 150; source: "images/music.png"; width_image: 100; height_image: media.checked ? 0 : 100
                        onClicked: applauncher.startApplication("mediaplayer")
                    }
                    ButtonControl{
                        x: 200; width: 150; height: media.checked ? 0 : 150; source: "images/map.png"; width_image: 100; height_image: media.checked ? 0 : 100
                        onClicked: applauncher.startApplication("navigation")
                    }
                    ButtonControl{
                        x: 375; y: -25; width: 200; height: media.checked ? 0 : 200; source: "images/youtube.png"; width_image: 100; height_image: media.checked ? 0 : 100
                        onClicked: applauncher.startApplication("social")
                    }
                    ButtonControl{
                        x: 600; width: 150; height: media.checked ? 0 : 150; source: "images/internet.png"; width_image: 100; height_image: media.checked ? 0 : 100
                        onClicked: applauncher.startApplication("social")
                    }
                    Behavior on height{NumberAnimation{duration: 300; easing.type: Easing.Linear}}
                }

                ButtonControl{
                    id: media_; y: rect1.height + 50; width: 50; height: 50; source: "images/arrow-" + (checked ? "down" : "top") +".png"; width_image: 24; height_image: 24
                }
                Text{
                    x: 50; y: rect1.height + 50; anchors.verticalCenter: media_.verticalCenter; font.family: fontLoader.name; font.pixelSize: 25; color: "white"
                    text: "Phương tiện"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignLeft
                }

                Rectangle{
                    x: 50; y: rect1.height + 100; width: parent.width; height: media_.checked ? 0 : 150; color: "transparent"
                    ButtonControl{
                        width: 150; height: media_.checked ? 0 : 150; source: "images/settings.png"; width_image: 100; height_image: media_.checked ? 0 : 100
                        onClicked: applauncher.startApplication("settings")
                    }
                }
        }
    }
}
