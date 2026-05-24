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
    width: 850
    height: 670
    color: "transparent"
    Rectangle{
        Background{width_:1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}
        y: 5; width: 860; height: 660; color: "#3f6f6f6f"; radius: 10

        FontLoader{
        id: fontLoader
        source: "images/Roboto-BoldCondensed.ttf"
        }
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
            BorderColor{borderWidth: 1; c1: "white"; c2: "white"}
        }

        View3D {
            anchors.fill: parent
            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
                clearColor: "transparent"
                lightProbe: Texture {   
                    source: "images/qwantani_mid_morning_puresky_1k.hdr"
                }
                antialiasingMode: SceneEnvironment.MSAA
                probeOrientation: Qt.vector3d(0, -220 - merce.rootNode.eulerRotation.y, 0)

                glowEnabled: true
                glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
                glowStrength: 1
                glowIntensity: 1
                glowBloom: 0.4
            }
            // OrbitCameraController{
            //     origin: merce.rootNode
            //     camera: merce.camera
            //     xInvert: true
            //     panEnabled: false
            //     ySpeed: 0.0
            // }
            Mercedes_Light{ id: merce;scale: Qt.vector3d(1, 1, 1)}
        }

        Text{
            x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
            text: "Điều khiển đèn"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            MultiEffect {
                anchors.fill: parent; source: parent; shadowEnabled: true
                shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
                shadowHorizontalOffset: 5; shadowVerticalOffset: 5
            }
        }

        Rectangle{
            x: 30; y: 100; width: 400; height: 100; color: "#3f6f6f6f"; radius: 20
            BorderColor{radius: 20}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 20
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 20}
            }
        }

        Rectangle{
            x: 30; y: 240; width: 100; height: 100; color: "#3f6f6f6f"; radius: 20
            BorderColor{radius: 20}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 20
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 20}
            }
        }

        Rectangle{
            x: 180; y: 240; width: 100; height: 100; color: "#3f6f6f6f"; radius: 20
            BorderColor{radius: 20}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 20
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 20}
            }
        }

        Rectangle{
            id: rect_
            visible: auto_light.checked || headlight.checked || taillight.checked || light_off.checked
            width: 100; height: 98; radius: 20// color: "blue";
            // border.color: Qt.darker(color, 1.2)
            Canvas {
                anchors.fill: parent
                antialiasing: true

                onPaint: {
                    const ctx = getContext("2d");
                    ctx.reset();

                    const r = 20; // radius (px) - chỉnh theo ý bạn

                    // Gradient ngang: trái -> phải
                    const g = ctx.createLinearGradient(0, 0, width, 0);
                    g.addColorStop(0.3, "black");
                    g.addColorStop(1.0, "#fa001b53");

                    // Rounded-rect path
                    const x = 0, y = 0, w = width, h = height;
                    const rr = Math.max(0, Math.min(r, Math.min(w, h) / 2));

                    ctx.beginPath();
                    ctx.moveTo(x + rr, y);
                    ctx.lineTo(x + w - rr, y);
                    ctx.quadraticCurveTo(x + w, y, x + w, y + rr);
                    ctx.lineTo(x + w, y + h - rr);
                    ctx.quadraticCurveTo(x + w, y + h, x + w - rr, y + h);
                    ctx.lineTo(x + rr, y + h);
                    ctx.quadraticCurveTo(x, y + h, x, y + h - rr);
                    ctx.lineTo(x, y + rr);
                    ctx.quadraticCurveTo(x, y, x + rr, y);
                    ctx.closePath();

                    ctx.fillStyle = g;
                    ctx.fill();
                }
            }
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0; verticalOffset: 0; radius: 20
                samples: 20; color: "#80FFFFFF"
            }
        }

        Rectangle{
            id: rect_fog_l
            visible: fog_l.checked
            width: 100; height: 98; radius: 20// color: "blue";
            // border.color: Qt.darker(color, 1.2)
            Canvas {
                anchors.fill: parent
                antialiasing: true

                onPaint: {
                    const ctx = getContext("2d");
                    ctx.reset();

                    const r = 20; // radius (px) - chỉnh theo ý bạn

                    // Gradient ngang: trái -> phải
                    const g = ctx.createLinearGradient(0, 0, width, 0);
                    g.addColorStop(0.3, "black");
                    g.addColorStop(1.0, "#fa001b53");

                    // Rounded-rect path
                    const x = 0, y = 0, w = width, h = height;
                    const rr = Math.max(0, Math.min(r, Math.min(w, h) / 2));

                    ctx.beginPath();
                    ctx.moveTo(x + rr, y);
                    ctx.lineTo(x + w - rr, y);
                    ctx.quadraticCurveTo(x + w, y, x + w, y + rr);
                    ctx.lineTo(x + w, y + h - rr);
                    ctx.quadraticCurveTo(x + w, y + h, x + w - rr, y + h);
                    ctx.lineTo(x + rr, y + h);
                    ctx.quadraticCurveTo(x, y + h, x, y + h - rr);
                    ctx.lineTo(x, y + rr);
                    ctx.quadraticCurveTo(x, y, x + rr, y);
                    ctx.closePath();

                    ctx.fillStyle = g;
                    ctx.fill();
                }
            }
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0; verticalOffset: 0; radius: 20
                samples: 20; color: "#80FFFFFF"
            }
        }

        Rectangle{
            id: rect_fog_r
            visible: fog_r.checked
            width: 100; height: 98; radius: 20// color: "blue";
            // border.color: Qt.darker(color, 1.2)
            Canvas {
                anchors.fill: parent
                antialiasing: true

                onPaint: {
                    const ctx = getContext("2d");
                    ctx.reset();

                    const r = 20; // radius (px) - chỉnh theo ý bạn

                    // Gradient ngang: trái -> phải
                    const g = ctx.createLinearGradient(0, 0, width, 0);
                    g.addColorStop(0.3, "black");
                    g.addColorStop(1.0, "#fa001b53");

                    // Rounded-rect path
                    const x = 0, y = 0, w = width, h = height;
                    const rr = Math.max(0, Math.min(r, Math.min(w, h) / 2));

                    ctx.beginPath();
                    ctx.moveTo(x + rr, y);
                    ctx.lineTo(x + w - rr, y);
                    ctx.quadraticCurveTo(x + w, y, x + w, y + rr);
                    ctx.lineTo(x + w, y + h - rr);
                    ctx.quadraticCurveTo(x + w, y + h, x + w - rr, y + h);
                    ctx.lineTo(x + rr, y + h);
                    ctx.quadraticCurveTo(x, y + h, x, y + h - rr);
                    ctx.lineTo(x, y + rr);
                    ctx.quadraticCurveTo(x, y, x + rr, y);
                    ctx.closePath();

                    ctx.fillStyle = g;
                    ctx.fill();
                }
            }
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0; verticalOffset: 0; radius: 20
                samples: 20; color: "#80FFFFFF"
            }
        }

        Row{
            x: 30; y: 100; width: 400; height: 100; spacing: 0
            ButtonControl {
                id: auto_light
                width: 100; height: 100; source: "images/light_auto.png"; width_image: 50; height_image: 50
                onClicked:{rect_.x = 30; rect_.y = 100; headlight.checked = true; taillight.checked = true; light_off.checked = true}
            }
            ButtonControl {
                id: headlight
                width: 100; height: 100; source: "images/headlight.png"; width_image: 50; height_image: 50
                onClicked:{rect_.x = 130; rect_.y = 100; auto_light.checked = true; taillight.checked = true; light_off.checked = true}
            }
            ButtonControl {
                id: taillight
                width: 100; height: 100; source: "images/taillight.png"; width_image: 50; height_image: 50
                onClicked:{rect_.x = 230; rect_.y = 100; headlight.checked = true; auto_light.checked = true; light_off.checked = true}
            }
            ButtonControl {
                id: light_off
                width: 100; height: 100; source: "images/light_off.png"; width_image: 50; height_image: 50
                onClicked:{rect_.x = 330; rect_.y = 100; headlight.checked = true; taillight.checked = true; auto_light.checked = true}
            }
        }

        ButtonControl {
            id: fog_l
            x: 30; y: 240; width: 100; height: 100; source: "images/fog_light_l.png"; width_image: 50; height_image: 50
            onClicked:{rect_fog_l.x = 30; rect_fog_l.y = 240; steering.checked = true; mirror.checked = true; glass.checked = true}
        }

        ButtonControl {
            id: fog_r
            x: 180; y: 240; width: 100; height: 100; source: "images/fog_light_r.png"; width_image: 50; height_image: 50
            onClicked:{rect_fog_r.x = 180; rect_fog_r.y = 240; steering.checked = true; mirror.checked = true; glass.checked = true}
        }

        ButtonControl {
            id: settings
            x: 30; y: 550; width: 80; height: 80; source: "images/settings.png"; width_image: 30; height_image: 30
        }
    }
}