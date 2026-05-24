import QtQuick
import QtQuick3D
import QtQml

Node {
    id: node
    scale.x: 10
    scale.y: 10
    scale.z: 10

    property alias rootNode: rootNode
    property alias activeCamera1: perspectiveCamera
    property alias activeCamera2: perspectiveCamera1
    property alias timelineAnimation: timelineAnimation
    property alias keyFrame: keyFrame
    property bool autoRotate: false
    property bool highQuality: true
    property string assetRootUrl: typeof externalAssetRootUrl === "undefined" ? "qrc:/asset/" : externalAssetRootUrl

    function assetUrl(path) {
        return assetRootUrl + path
    }

    QtObject {
        id: keyFrame
        property real value: 360

        onValueChanged: {
            rootNode.eulerRotation = Qt.vector3d(rootNode.eulerRotation.x, value, rootNode.eulerRotation.z)
        }
    }

    // Resources
    property url textureData: node.assetUrl("maps/textureData.png")
    property url textureData313: node.assetUrl("maps/textureData313.png")
    property url textureData511: node.assetUrl("maps/textureData511.png")
    property url textureData9: node.assetUrl("maps/textureData9.png")
    property url textureData333: node.assetUrl("maps/textureData333.png")
    property url textureData11: node.assetUrl("maps/textureData11.png")
    property url textureData311: node.assetUrl("maps/textureData311.png")
    property url textureData299: node.assetUrl("maps/textureData299.png")
    property url textureData294: node.assetUrl("maps/textureData294.png")
    property url textureData16: node.assetUrl("maps/textureData16.png")
    property url textureData449: node.assetUrl("maps/textureData449.png")
    property url textureData18: node.assetUrl("maps/textureData18.png")
    property url textureData451: node.assetUrl("maps/textureData451.png")
    property url textureData301: node.assetUrl("maps/textureData301.png")
    property url textureData346: node.assetUrl("maps/textureData346.jpg")
    property url textureData453: node.assetUrl("maps/textureData453.png")
    property url textureData509: node.assetUrl("maps/textureData509.png")

    // Nodes:
    Node {
        id: sketchfab_model
        objectName: "Sketchfab_model"
        rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
        Node {
            id: crown_2016_fbx
            objectName: "crown_2016.fbx"
            rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
            scale.x: 0.01
            scale.y: 0.01
            scale.z: 0.01
            Node {
                id: rootNode
                objectName: "RootNode"
                NumberAnimation on eulerRotation.y {
                    id: timelineAnimation
                    from: 0
                    to: 360
                    duration: 18000
                    loops: Animation.Infinite
                    running: node.autoRotate
                }

                Node {
                    id: s210_tailgatelight_L
                    objectName: "s210_tailgatelight_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_tailgatelight_L_s210_revik_0
                        objectName: "s210_tailgatelight_L_s210_revik_0"
                        source: node.assetUrl("meshes/s210_tailgatelight_L_s210_revik_0_mesh.mesh")
                        materials: [
                            s210_revik_material
                        ]
                    }
                    Model {
                        id: s210_tailgatelight_L_s210_lights1_0
                        objectName: "s210_tailgatelight_L_s210_lights1_0"
                        source: node.assetUrl("meshes/s210_tailgatelight_L_s210_lights1_0_mesh.mesh")
                        materials: [
                            s210_lights1_material
                        ]
                    }
                    Model {
                        id: s210_tailgatelight_L_s210_lowhi_0
                        objectName: "s210_tailgatelight_L_s210_lowhi_0"
                        source: node.assetUrl("meshes/s210_tailgatelight_L_s210_lowhi_0_mesh.mesh")
                        materials: [
                            s210_lowhi_material
                        ]
                    }
                }
                Node {
                    id: s210_taillight_L
                    objectName: "s210_taillight_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_taillight_L_s210_revik_0
                        objectName: "s210_taillight_L_s210_revik_0"
                        source: node.assetUrl("meshes/s210_taillight_L_s210_revik_0_mesh.mesh")
                        materials: [
                            s210_revik_material
                        ]
                    }
                    Model {
                        id: s210_taillight_L_s210_lights1_0
                        objectName: "s210_taillight_L_s210_lights1_0"
                        source: node.assetUrl("meshes/s210_taillight_L_s210_lights1_0_mesh.mesh")
                        materials: [
                            s210_lights1_material
                        ]
                    }
                    Model {
                        id: s210_taillight_L_s210_lowhi_0
                        objectName: "s210_taillight_L_s210_lowhi_0"
                        source: node.assetUrl("meshes/s210_taillight_L_s210_lowhi_0_mesh.mesh")
                        materials: [
                            s210_lowhi_material
                        ]
                    }
                }
                Node {
                    id: s210_tailgatelight_R
                    objectName: "s210_tailgatelight_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_tailgatelight_R_s210_revik_0
                        objectName: "s210_tailgatelight_R_s210_revik_0"
                        source: node.assetUrl("meshes/s210_tailgatelight_R_s210_revik_0_mesh.mesh")
                        materials: [
                            s210_revik_material
                        ]
                    }
                    Model {
                        id: s210_tailgatelight_R_s210_lights1_0
                        objectName: "s210_tailgatelight_R_s210_lights1_0"
                        source: node.assetUrl("meshes/s210_tailgatelight_R_s210_lights1_0_mesh.mesh")
                        materials: [
                            s210_lights1_material
                        ]
                    }
                    Model {
                        id: s210_tailgatelight_R_s210_lowhi_0
                        objectName: "s210_tailgatelight_R_s210_lowhi_0"
                        source: node.assetUrl("meshes/s210_tailgatelight_R_s210_lowhi_0_mesh.mesh")
                        materials: [
                            s210_lowhi_material
                        ]
                    }
                }
                Node {
                    id: s210_tailgatelightglass_R
                    objectName: "s210_tailgatelightglass_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_tailgatelightglass_R_s210_glass_0
                        objectName: "s210_tailgatelightglass_R_s210_glass_0"
                        source: node.assetUrl("meshes/s210_tailgatelightglass_R_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                    Model {
                        id: s210_tailgatelightglass_R_s210_lights_glass_0
                        objectName: "s210_tailgatelightglass_R_s210_lights_glass_0"
                        source: node.assetUrl("meshes/s210_tailgatelightglass_R_s210_lights_glass_0_mesh.mesh")
                        materials: [
                            s210_lights_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_taillight_R
                    objectName: "s210_taillight_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_taillight_R_s210_revik_0
                        objectName: "s210_taillight_R_s210_revik_0"
                        source: node.assetUrl("meshes/s210_taillight_R_s210_revik_0_mesh.mesh")
                        materials: [
                            s210_revik_material
                        ]
                    }
                    Model {
                        id: s210_taillight_R_s210_lights1_0
                        objectName: "s210_taillight_R_s210_lights1_0"
                        source: node.assetUrl("meshes/s210_taillight_R_s210_lights1_0_mesh.mesh")
                        materials: [
                            s210_lights1_material
                        ]
                    }
                    Model {
                        id: s210_taillight_R_s210_lowhi_0
                        objectName: "s210_taillight_R_s210_lowhi_0"
                        source: node.assetUrl("meshes/s210_taillight_R_s210_lowhi_0_mesh.mesh")
                        materials: [
                            s210_lowhi_material
                        ]
                    }
                }
                Node {
                    id: s210_taillightglass_R
                    objectName: "s210_taillightglass_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_taillightglass_R_s210_glass_0
                        objectName: "s210_taillightglass_R_s210_glass_0"
                        source: node.assetUrl("meshes/s210_taillightglass_R_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                    Model {
                        id: s210_taillightglass_R_s210_lights_glass_0
                        objectName: "s210_taillightglass_R_s210_lights_glass_0"
                        source: node.assetUrl("meshes/s210_taillightglass_R_s210_lights_glass_0_mesh.mesh")
                        materials: [
                            s210_lights_glass_material
                        ]
                    }
                }
                Node {
                    id: body_001
                    objectName: "BODY.001"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: body_001_s210_block_0
                        objectName: "BODY.001_s210_block_0"
                        source: node.assetUrl("meshes/body_001_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: body_001_etk_wheel_03a_0
                        objectName: "BODY.001_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/body_001_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: body_004
                    objectName: "BODY.004"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: body_004_s210_block_0
                        objectName: "BODY.004_s210_block_0"
                        source: node.assetUrl("meshes/body_004_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: body_004_etk_wheel_03a_0
                        objectName: "BODY.004_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/body_004_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: s210_bumper_R_g
                    objectName: "s210_bumper_R_g"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_bumper_R_g_s210_body_0
                        objectName: "s210_bumper_R_g_s210_body_0"
                        source: node.assetUrl("meshes/s210_bumper_R_g_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_bumper_R_g_s210_block_0
                        objectName: "s210_bumper_R_g_s210_block_0"
                        source: node.assetUrl("meshes/s210_bumper_R_g_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_lettering_athlete_g
                    objectName: "s210_lettering_athlete_g"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_lettering_athlete_g_wheel_42a_0
                        objectName: "s210_lettering_athlete_g_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_lettering_athlete_g_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                }
                Node {
                    id: s210_engine_starter
                    objectName: "s210_engine_starter"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_engine_starter_bastion_engine_0
                        objectName: "s210_engine_starter_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_engine_starter_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_map_sensor_wire
                    objectName: "s210_map_sensor_wire"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_map_sensor_wire_bastion_engine_0
                        objectName: "s210_map_sensor_wire_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_map_sensor_wire_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_accessory_belt_supercharged
                    objectName: "s210_v6_accessory_belt_supercharged"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_accessory_belt_supercharged_bastion_engine_0
                        objectName: "s210_v6_accessory_belt_supercharged_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_accessory_belt_supercharged_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_supercharger_pulley
                    objectName: "s210_v6_supercharger_pulley"
                    x: -5.711513996124268
                    y: 77.95509338378906
                    z: 177.82550048828125
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_supercharger_pulley_bastion_engine_0
                        objectName: "s210_v6_supercharger_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_supercharger_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_block
                    objectName: "s210_v6_block"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_block_bastion_engine_0
                        objectName: "s210_v6_block_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_block_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_ac_belt
                    objectName: "s210_v6_ac_belt"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_ac_belt_bastion_engine_0
                        objectName: "s210_v6_ac_belt_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_ac_belt_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_alternator_pulley
                    objectName: "s210_v6_alternator_pulley"
                    x: 26.43144989013672
                    y: 43.03389358520508
                    z: 178.2637939453125
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_alternator_pulley_bastion_engine_0
                        objectName: "s210_v6_alternator_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_alternator_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_belt_tensioner_pulley
                    objectName: "s210_v6_belt_tensioner_pulley"
                    x: 16.33698081970215
                    y: 52.35588073730469
                    z: 178.2637939453125
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_belt_tensioner_pulley_bastion_engine_0
                        objectName: "s210_v6_belt_tensioner_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_belt_tensioner_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_crankshaft_pulley
                    objectName: "s210_v6_crankshaft_pulley"
                    x: -0.0003639610076788813
                    y: 42.88085174560547
                    z: 176.9757080078125
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_crankshaft_pulley_bastion_engine_0
                        objectName: "s210_v6_crankshaft_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_crankshaft_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_steering_pump_pulley
                    objectName: "s210_v6_steering_pump_pulley"
                    x: 27.06715202331543
                    y: 64.96503448486328
                    z: 179.0699920654297
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_steering_pump_pulley_bastion_engine_0
                        objectName: "s210_v6_steering_pump_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_steering_pump_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_water_pump_pulley
                    objectName: "s210_v6_water_pump_pulley"
                    x: -0.0003647059784270823
                    y: 64.20729064941406
                    z: 179.55050659179688
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_water_pump_pulley_bastion_engine_0
                        objectName: "s210_v6_water_pump_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_water_pump_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_exhaust_header_R
                    objectName: "s210_v6_exhaust_header_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_exhaust_header_R_bastion_engine_0
                        objectName: "s210_v6_exhaust_header_R_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_exhaust_header_R_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_accessories
                    objectName: "s210_v6_accessories"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_accessories_bastion_engine_0
                        objectName: "s210_v6_accessories_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_accessories_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_ac_belt_tensioner_pulley
                    objectName: "s210_v6_ac_belt_tensioner_pulley"
                    x: -14.710350036621094
                    y: 35.965938568115234
                    z: 173.60899353027344
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_ac_belt_tensioner_pulley_bastion_engine_0
                        objectName: "s210_v6_ac_belt_tensioner_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_ac_belt_tensioner_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_ac_pump_pulley
                    objectName: "s210_v6_ac_pump_pulley"
                    x: -26.297840118408203
                    y: 42.47660827636719
                    z: 176.00250244140625
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_ac_pump_pulley_bastion_engine_0
                        objectName: "s210_v6_ac_pump_pulley_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_ac_pump_pulley_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_v6_exhaust_header_L
                    objectName: "s210_v6_exhaust_header_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_v6_exhaust_header_L_bastion_engine_0
                        objectName: "s210_v6_exhaust_header_L_bastion_engine_0"
                        source: node.assetUrl("meshes/s210_v6_exhaust_header_L_bastion_engine_0_mesh.mesh")
                        materials: [
                            bastion_engine_material
                        ]
                    }
                }
                Node {
                    id: s210_body_002
                    objectName: "s210_body.002"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_body_002_s210_body_0
                        objectName: "s210_body.002_s210_body_0"
                        source: node.assetUrl("meshes/s210_body_002_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                }
                Node {
                    id: s210_backlight_sedan_001
                    objectName: "s210_backlight_sedan.001"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_backlight_sedan_001_s210_glass_0
                        objectName: "s210_backlight_sedan.001_s210_glass_0"
                        source: node.assetUrl("meshes/s210_backlight_sedan_001_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_RL_001
                    objectName: "s210_doorglass_RL.001"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_RL_001_s210_glass_0
                        objectName: "s210_doorglass_RL.001_s210_glass_0"
                        source: node.assetUrl("meshes/s210_doorglass_RL_001_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_windshield_001
                    objectName: "s210_windshield.001"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_windshield_001_s210_glass_0
                        objectName: "s210_windshield.001_s210_glass_0"
                        source: node.assetUrl("meshes/s210_windshield_001_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_banka_stock_L
                    objectName: "s210_banka_stock_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_banka_stock_L_s210_0
                        objectName: "s210_banka_stock_L_s210_0"
                        source: node.assetUrl("meshes/s210_banka_stock_L_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_underbody
                    objectName: "s210_underbody"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_underbody_s210_0
                        objectName: "s210_underbody_s210_0"
                        source: node.assetUrl("meshes/s210_underbody_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_driveshaft_F
                    objectName: "s210_driveshaft_F"
                    x: 14.365230560302734
                    y: 33.439998626708984
                    z: 86.35284423828125
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_driveshaft_F_s210_0
                        objectName: "s210_driveshaft_F_s210_0"
                        source: node.assetUrl("meshes/s210_driveshaft_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_icpipe_i_i4
                    objectName: "s210_icpipe_i_i4"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_icpipe_i_i4_etk_engine_i6_0
                        objectName: "s210_icpipe_i_i4_etk_engine_i6_0"
                        source: node.assetUrl("meshes/s210_icpipe_i_i4_etk_engine_i6_0_mesh.mesh")
                        materials: [
                            etk_engine_i6_material
                        ]
                    }
                }
                Node {
                    id: s210_icpipe_t_i4
                    objectName: "s210_icpipe_t_i4"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_icpipe_t_i4_etk_engine_i6_0
                        objectName: "s210_icpipe_t_i4_etk_engine_i6_0"
                        source: node.assetUrl("meshes/s210_icpipe_t_i4_etk_engine_i6_0_mesh.mesh")
                        materials: [
                            etk_engine_i6_material
                        ]
                    }
                }
                Node {
                    id: s210_radtube_i4
                    objectName: "s210_radtube_i4"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_radtube_i4_etk_engine_i6_0
                        objectName: "s210_radtube_i4_etk_engine_i6_0"
                        source: node.assetUrl("meshes/s210_radtube_i4_etk_engine_i6_0_mesh.mesh")
                        materials: [
                            etk_engine_i6_material
                        ]
                    }
                }
                Node {
                    id: s210_radtube_i6
                    objectName: "s210_radtube_i6"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_radtube_i6_etk_engine_i6_0
                        objectName: "s210_radtube_i6_etk_engine_i6_0"
                        source: node.assetUrl("meshes/s210_radtube_i6_etk_engine_i6_0_mesh.mesh")
                        materials: [
                            etk_engine_i6_material
                        ]
                    }
                }
                Node {
                    id: s210_icpipe_t_i6
                    objectName: "s210_icpipe_t_i6"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_icpipe_t_i6_etk_engine_i6_0
                        objectName: "s210_icpipe_t_i6_etk_engine_i6_0"
                        source: node.assetUrl("meshes/s210_icpipe_t_i6_etk_engine_i6_0_mesh.mesh")
                        materials: [
                            etk_engine_i6_material
                        ]
                    }
                }
                Node {
                    id: s210_icpipe_i_i6
                    objectName: "s210_icpipe_i_i6"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_icpipe_i_i6_etk_engine_i6_0
                        objectName: "s210_icpipe_i_i6_etk_engine_i6_0"
                        source: node.assetUrl("meshes/s210_icpipe_i_i6_etk_engine_i6_0_mesh.mesh")
                        materials: [
                            etk_engine_i6_material
                        ]
                    }
                }
                Node {
                    id: s210_swaybar_F
                    objectName: "s210_swaybar_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_swaybar_F_s210_0
                        objectName: "s210_swaybar_F_s210_0"
                        source: node.assetUrl("meshes/s210_swaybar_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_transmission
                    objectName: "s210_transmission"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_transmission_s210_0
                        objectName: "s210_transmission_s210_0"
                        source: node.assetUrl("meshes/s210_transmission_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_transfercase
                    objectName: "s210_transfercase"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_transfercase_s210_0
                        objectName: "s210_transfercase_s210_0"
                        source: node.assetUrl("meshes/s210_transfercase_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_diff_F
                    objectName: "s210_diff_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_diff_F_s210_0
                        objectName: "s210_diff_F_s210_0"
                        source: node.assetUrl("meshes/s210_diff_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_exhaust_R
                    objectName: "s210_exhaust_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_exhaust_R_s210_0
                        objectName: "s210_exhaust_R_s210_0"
                        source: node.assetUrl("meshes/s210_exhaust_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_intercooler
                    objectName: "s210_intercooler"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_intercooler_s210_0
                        objectName: "s210_intercooler_s210_0"
                        source: node.assetUrl("meshes/s210_intercooler_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_radiator
                    objectName: "s210_radiator"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_radiator_s210_0
                        objectName: "s210_radiator_s210_0"
                        source: node.assetUrl("meshes/s210_radiator_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_lowerarm_F_a
                    objectName: "s210_lowerarm_F_a"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_lowerarm_F_a_s210_0
                        objectName: "s210_lowerarm_F_a_s210_0"
                        source: node.assetUrl("meshes/s210_lowerarm_F_a_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_heatshield
                    objectName: "s210_heatshield"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_heatshield_s210_0
                        objectName: "s210_heatshield_s210_0"
                        source: node.assetUrl("meshes/s210_heatshield_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_swaybar_R
                    objectName: "s210_swaybar_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_swaybar_R_s210_0
                        objectName: "s210_swaybar_R_s210_0"
                        source: node.assetUrl("meshes/s210_swaybar_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_diff
                    objectName: "s210_diff"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_diff_s210_0
                        objectName: "s210_diff_s210_0"
                        source: node.assetUrl("meshes/s210_diff_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_exhaust_L_b
                    objectName: "s210_exhaust_L_b"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_exhaust_L_b_s210_0
                        objectName: "s210_exhaust_L_b_s210_0"
                        source: node.assetUrl("meshes/s210_exhaust_L_b_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_exhaust_L
                    objectName: "s210_exhaust_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_exhaust_L_s210_0
                        objectName: "s210_exhaust_L_s210_0"
                        source: node.assetUrl("meshes/s210_exhaust_L_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_driveshaft
                    objectName: "s210_driveshaft"
                    x: -2.0623199816327542e-05
                    y: 34.660987854003906
                    z: -42.076210021972656
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_driveshaft_s210_0
                        objectName: "s210_driveshaft_s210_0"
                        source: node.assetUrl("meshes/s210_driveshaft_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_fueltank
                    objectName: "s210_fueltank"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_fueltank_s210_0
                        objectName: "s210_fueltank_s210_0"
                        source: node.assetUrl("meshes/s210_fueltank_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_tierod_F
                    objectName: "s210_tierod_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_tierod_F_s210_0
                        objectName: "s210_tierod_F_s210_0"
                        source: node.assetUrl("meshes/s210_tierod_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_steeringbox
                    objectName: "s210_steeringbox"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_steeringbox_s210_0
                        objectName: "s210_steeringbox_s210_0"
                        source: node.assetUrl("meshes/s210_steeringbox_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_spring_R
                    objectName: "s210_spring_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_spring_R_s210_0
                        objectName: "s210_spring_R_s210_0"
                        source: node.assetUrl("meshes/s210_spring_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_tierod_R
                    objectName: "s210_tierod_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_tierod_R_s210_0
                        objectName: "s210_tierod_R_s210_0"
                        source: node.assetUrl("meshes/s210_tierod_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_upperarm_R
                    objectName: "s210_upperarm_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_upperarm_R_s210_0
                        objectName: "s210_upperarm_R_s210_0"
                        source: node.assetUrl("meshes/s210_upperarm_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_lowerarm_R
                    objectName: "s210_lowerarm_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_lowerarm_R_s210_0
                        objectName: "s210_lowerarm_R_s210_0"
                        source: node.assetUrl("meshes/s210_lowerarm_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_radsupport
                    objectName: "s210_radsupport"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_radsupport_s210_0
                        objectName: "s210_radsupport_s210_0"
                        source: node.assetUrl("meshes/s210_radsupport_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_hub_R
                    objectName: "s210_hub_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_hub_R_s210_0
                        objectName: "s210_hub_R_s210_0"
                        source: node.assetUrl("meshes/s210_hub_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_subframe_R
                    objectName: "s210_subframe_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_subframe_R_s210_0
                        objectName: "s210_subframe_R_s210_0"
                        source: node.assetUrl("meshes/s210_subframe_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_strut_F
                    objectName: "s210_strut_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_strut_F_s210_0
                        objectName: "s210_strut_F_s210_0"
                        source: node.assetUrl("meshes/s210_strut_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_hub_F
                    objectName: "s210_hub_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_hub_F_s210_0
                        objectName: "s210_hub_F_s210_0"
                        source: node.assetUrl("meshes/s210_hub_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_halfshaft_F
                    objectName: "s210_halfshaft_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_halfshaft_F_s210_0
                        objectName: "s210_halfshaft_F_s210_0"
                        source: node.assetUrl("meshes/s210_halfshaft_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_halfshaft_R
                    objectName: "s210_halfshaft_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_halfshaft_R_s210_0
                        objectName: "s210_halfshaft_R_s210_0"
                        source: node.assetUrl("meshes/s210_halfshaft_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_subframe_F
                    objectName: "s210_subframe_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_subframe_F_s210_0
                        objectName: "s210_subframe_F_s210_0"
                        source: node.assetUrl("meshes/s210_subframe_F_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_lowerarm_F_b
                    objectName: "s210_lowerarm_F_b"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_lowerarm_F_b_s210_0
                        objectName: "s210_lowerarm_F_b_s210_0"
                        source: node.assetUrl("meshes/s210_lowerarm_F_b_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_shock_R
                    objectName: "s210_shock_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    Model {
                        id: s210_shock_R_s210_0
                        objectName: "s210_shock_R_s210_0"
                        source: node.assetUrl("meshes/s210_shock_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_taillightglass_L
                    objectName: "s210_taillightglass_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_taillightglass_L_s210_glass_0
                        objectName: "s210_taillightglass_L_s210_glass_0"
                        source: node.assetUrl("meshes/s210_taillightglass_L_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                    Model {
                        id: s210_taillightglass_L_s210_lights_glass_0
                        objectName: "s210_taillightglass_L_s210_lights_glass_0"
                        source: node.assetUrl("meshes/s210_taillightglass_L_s210_lights_glass_0_mesh.mesh")
                        materials: [
                            s210_lights_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_wheel_18x8
                    objectName: "s210_wheel_18x8"
                    x: 74.40277099609375
                    y: 38.29964065551758
                    z: 140.20240783691406
                    rotation: Qt.quaternion(0.706434, -0.706434, -0.0308436, 0.0308436)
                    scale.x: 122.624
                    scale.y: 122.624
                    scale.z: 122.624
                    Model {
                        id: s210_wheel_18x8_wheel_42a_0
                        objectName: "s210_wheel_18x8_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_s210_block_0
                        objectName: "s210_wheel_18x8_s210_block_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_etk_wheel_03a_0
                        objectName: "s210_wheel_18x8_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: s210_tailgatelightglass_L
                    objectName: "s210_tailgatelightglass_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_tailgatelightglass_L_s210_glass_0
                        objectName: "s210_tailgatelightglass_L_s210_glass_0"
                        source: node.assetUrl("meshes/s210_tailgatelightglass_L_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                    Model {
                        id: s210_tailgatelightglass_L_s210_lights_glass_0
                        objectName: "s210_tailgatelightglass_L_s210_lights_glass_0"
                        source: node.assetUrl("meshes/s210_tailgatelightglass_L_s210_lights_glass_0_mesh.mesh")
                        materials: [
                            s210_lights_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_body
                    objectName: "s210_body"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_body_s210_body_0
                        objectName: "s210_body_s210_body_0"
                        source: node.assetUrl("meshes/s210_body_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_body_wheel_42a_0
                        objectName: "s210_body_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_body_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_body_s210_block_0
                        objectName: "s210_body_s210_block_0"
                        source: node.assetUrl("meshes/s210_body_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_body_s210_interior_LOD0_0
                        objectName: "s210_body_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_body_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                }
                Node {
                    id: s210_cockpit
                    objectName: "s210_cockpit"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_cockpit_s210_block_0
                        objectName: "s210_cockpit_s210_block_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_cockpit_s210_leather_white_512_0
                        objectName: "s210_cockpit_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                    Model {
                        id: s210_cockpit_s210_carpet_0
                        objectName: "s210_cockpit_s210_carpet_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_carpet_0_mesh.mesh")
                        materials: [
                            s210_carpet_material
                        ]
                    }
                    Model {
                        id: s210_cockpit_s210_leatherdark_0
                        objectName: "s210_cockpit_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_cockpit_s210_seatbelt_001_0
                        objectName: "s210_cockpit_s210_seatbelt_001_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_seatbelt_001_0_mesh.mesh")
                        materials: [
                            s210_seatbelt_001_material
                        ]
                    }
                    Model {
                        id: s210_cockpit_s210_interior_LOD0_0
                        objectName: "s210_cockpit_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_cockpit_s210_sperm_0
                        objectName: "s210_cockpit_s210_sperm_0"
                        source: node.assetUrl("meshes/s210_cockpit_s210_sperm_0_mesh.mesh")
                        materials: [
                            s210_sperm_material
                        ]
                    }
                }
                Node {
                    id: s210_dash
                    objectName: "s210_dash"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_dash_etk_wheel_03a_0
                        objectName: "s210_dash_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_dash_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_block_0
                        objectName: "s210_dash_s210_block_0"
                        source: node.assetUrl("meshes/s210_dash_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_leatherdark_0
                        objectName: "s210_dash_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_dash_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_wood_057_diff_0
                        objectName: "s210_dash_s210_wood_057_diff_0"
                        source: node.assetUrl("meshes/s210_dash_s210_wood_057_diff_0_mesh.mesh")
                        materials: [
                            s210_wood_057_diff_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_dd_0
                        objectName: "s210_dash_s210_dd_0"
                        source: node.assetUrl("meshes/s210_dash_s210_dd_0_mesh.mesh")
                        materials: [
                            s210_dd_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_interior_LOD0_0
                        objectName: "s210_dash_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_dash_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_screen_0
                        objectName: "s210_dash_s210_screen_0"
                        source: node.assetUrl("meshes/s210_dash_s210_screen_0_mesh.mesh")
                        materials: [
                            s210_screen_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_numbers_0
                        objectName: "s210_dash_s210_numbers_0"
                        source: node.assetUrl("meshes/s210_dash_s210_numbers_0_mesh.mesh")
                        materials: [
                            s210_numbers_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_comp_0
                        objectName: "s210_dash_s210_comp_0"
                        source: node.assetUrl("meshes/s210_dash_s210_comp_0_mesh.mesh")
                        materials: [
                            s210_comp_material
                        ]
                    }
                    Model {
                        id: s210_dash_s210_noski_0
                        objectName: "s210_dash_s210_noski_0"
                        source: node.assetUrl("meshes/s210_dash_s210_noski_0_mesh.mesh")
                        materials: [
                            s210_noski_material
                        ]
                    }
                }
                Node {
                    id: s210_grille
                    objectName: "s210_grille"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_grille_wheel_42a_0
                        objectName: "s210_grille_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_grille_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_grille_s210_block_0
                        objectName: "s210_grille_s210_block_0"
                        source: node.assetUrl("meshes/s210_grille_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_grille_s210_block_0355
                        objectName: "s210_grille_s210_block_0"
                        source: node.assetUrl("meshes/s210_grille_s210_block_0_mesh356.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_bumper_F_a
                    objectName: "s210_bumper_F_a"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_bumper_F_a_s210_body_0
                        objectName: "s210_bumper_F_a_s210_body_0"
                        source: node.assetUrl("meshes/s210_bumper_F_a_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_bumper_F_a_s210_block_0
                        objectName: "s210_bumper_F_a_s210_block_0"
                        source: node.assetUrl("meshes/s210_bumper_F_a_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_foglightglass_R_a
                    objectName: "s210_foglightglass_R_a"
                    y: 12.344419479370117
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_foglightglass_R_a_s210_glass_0
                        objectName: "s210_foglightglass_R_a_s210_glass_0"
                        source: node.assetUrl("meshes/s210_foglightglass_R_a_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_foglight_R_a
                    objectName: "s210_foglight_R_a"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_foglight_R_a_s210_fogl_0
                        objectName: "s210_foglight_R_a_s210_fogl_0"
                        source: node.assetUrl("meshes/s210_foglight_R_a_s210_fogl_0_mesh.mesh")
                        materials: [
                            s210_fogl_material
                        ]
                    }
                }
                Node {
                    id: s210_lip_F
                    objectName: "s210_lip_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_lip_F_s210_body_0
                        objectName: "s210_lip_F_s210_body_0"
                        source: node.assetUrl("meshes/s210_lip_F_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_lip_F_s210_runninglight_0
                        objectName: "s210_lip_F_s210_runninglight_0"
                        source: node.assetUrl("meshes/s210_lip_F_s210_runninglight_0_mesh.mesh")
                        materials: [
                            s210_runninglight_material
                        ]
                    }
                }
                Node {
                    id: s210_headlight_L
                    objectName: "s210_headlight_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_headlight_L_wheel_42a_0
                        objectName: "s210_headlight_L_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_headlight_L_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_headlight_L_s210_block_0
                        objectName: "s210_headlight_L_s210_block_0"
                        source: node.assetUrl("meshes/s210_headlight_L_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_headlight_L_s210_sigL_0
                        objectName: "s210_headlight_L_s210_sigL_0"
                        source: node.assetUrl("meshes/s210_headlight_L_s210_sigL_0_mesh.mesh")
                        materials: [
                            s210_sigL_material
                        ]
                    }
                    Model {
                        id: s210_headlight_L_s210_runninglight_0
                        objectName: "s210_headlight_L_s210_runninglight_0"
                        source: node.assetUrl("meshes/s210_headlight_L_s210_runninglight_0_mesh.mesh")
                        materials: [
                            s210_runninglight_material
                        ]
                    }
                    Model {
                        id: s210_headlight_L_s210_headlight_0
                        objectName: "s210_headlight_L_s210_headlight_0"
                        source: node.assetUrl("meshes/s210_headlight_L_s210_headlight_0_mesh.mesh")
                        materials: [
                            s210_headlight_material
                        ]
                    }
                }
                Node {
                    id: s210_headlightglass_R
                    objectName: "s210_headlightglass_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_headlightglass_R_s210_glass_0
                        objectName: "s210_headlightglass_R_s210_glass_0"
                        source: node.assetUrl("meshes/s210_headlightglass_R_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_engine
                    objectName: "s210_engine"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_engine_etk_wheel_03a_0
                        objectName: "s210_engine_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_engine_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_engine_s210_block_0
                        objectName: "s210_engine_s210_block_0"
                        source: node.assetUrl("meshes/s210_engine_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_hood
                    objectName: "s210_hood"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_hood_s210_body_0
                        objectName: "s210_hood_s210_body_0"
                        source: node.assetUrl("meshes/s210_hood_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_hood_wheel_42a_0
                        objectName: "s210_hood_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_hood_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_hood_s210_block_0
                        objectName: "s210_hood_s210_block_0"
                        source: node.assetUrl("meshes/s210_hood_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_trunk
                    objectName: "s210_trunk"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_trunk_s210_body_0
                        objectName: "s210_trunk_s210_body_0"
                        source: node.assetUrl("meshes/s210_trunk_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_trunk_wheel_42a_0
                        objectName: "s210_trunk_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_trunk_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_trunk_s210_block_0
                        objectName: "s210_trunk_s210_block_0"
                        source: node.assetUrl("meshes/s210_trunk_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_trunk_s210_leather_white_512_0
                        objectName: "s210_trunk_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_trunk_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                }
                Node {
                    id: s210_door_FR
                    objectName: "s210_door_FR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_door_FR_s210_body_0
                        objectName: "s210_door_FR_s210_body_0"
                        source: node.assetUrl("meshes/s210_door_FR_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_door_FR_wheel_42a_0
                        objectName: "s210_door_FR_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_door_FR_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_door_FR_s210_block_0
                        objectName: "s210_door_FR_s210_block_0"
                        source: node.assetUrl("meshes/s210_door_FR_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_door_FR_s210_leatherdark_0
                        objectName: "s210_door_FR_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_door_FR_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                }
                Node {
                    id: s210_door_FL
                    objectName: "s210_door_FL"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_door_FL_s210_body_0
                        objectName: "s210_door_FL_s210_body_0"
                        source: node.assetUrl("meshes/s210_door_FL_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_door_FL_wheel_42a_0
                        objectName: "s210_door_FL_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_door_FL_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_door_FL_s210_block_0
                        objectName: "s210_door_FL_s210_block_0"
                        source: node.assetUrl("meshes/s210_door_FL_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_door_FL_s210_leatherdark_0
                        objectName: "s210_door_FL_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_door_FL_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                }
                Node {
                    id: s210_door_RL
                    objectName: "s210_door_RL"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_door_RL_s210_body_0
                        objectName: "s210_door_RL_s210_body_0"
                        source: node.assetUrl("meshes/s210_door_RL_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_door_RL_wheel_42a_0
                        objectName: "s210_door_RL_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_door_RL_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_door_RL_s210_block_0
                        objectName: "s210_door_RL_s210_block_0"
                        source: node.assetUrl("meshes/s210_door_RL_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_door_RR
                    objectName: "s210_door_RR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_door_RR_s210_body_0
                        objectName: "s210_door_RR_s210_body_0"
                        source: node.assetUrl("meshes/s210_door_RR_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_door_RR_wheel_42a_0
                        objectName: "s210_door_RR_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_door_RR_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_door_RR_s210_block_0
                        objectName: "s210_door_RR_s210_block_0"
                        source: node.assetUrl("meshes/s210_door_RR_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_lettering_hybrid
                    objectName: "s210_lettering_hybrid"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_lettering_hybrid_s210_hyb_0
                        objectName: "s210_lettering_hybrid_s210_hyb_0"
                        source: node.assetUrl("meshes/s210_lettering_hybrid_s210_hyb_0_mesh.mesh")
                        materials: [
                            s210_hyb_material
                        ]
                    }
                }
                Node {
                    id: s210_lettering_athlete
                    objectName: "s210_lettering_athlete"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_lettering_athlete_wheel_42a_0
                        objectName: "s210_lettering_athlete_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_lettering_athlete_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                }
                Node {
                    id: s210_lettering_crown
                    objectName: "s210_lettering_crown"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_lettering_crown_wheel_42a_0
                        objectName: "s210_lettering_crown_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_lettering_crown_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                }
                Node {
                    id: s210_logo_R
                    objectName: "s210_logo_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_logo_R_wheel_42a_0
                        objectName: "s210_logo_R_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_logo_R_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_FL
                    objectName: "s210_doorglass_FL"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_FL_s210_glass_0
                        objectName: "s210_doorglass_FL_s210_glass_0"
                        source: node.assetUrl("meshes/s210_doorglass_FL_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_RR
                    objectName: "s210_doorglass_RR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_RR_s210_glass_0
                        objectName: "s210_doorglass_RR_s210_glass_0"
                        source: node.assetUrl("meshes/s210_doorglass_RR_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_FR
                    objectName: "s210_doorglass_FR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_FR_s210_glass_0
                        objectName: "s210_doorglass_FR_s210_glass_0"
                        source: node.assetUrl("meshes/s210_doorglass_FR_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_mirror_R
                    objectName: "s210_mirror_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_mirror_R_s210_body_0
                        objectName: "s210_mirror_R_s210_body_0"
                        source: node.assetUrl("meshes/s210_mirror_R_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_mirror_R_mirror_0
                        objectName: "s210_mirror_R_mirror_0"
                        source: node.assetUrl("meshes/s210_mirror_R_mirror_0_mesh.mesh")
                        materials: [
                            mirror_material
                        ]
                    }
                    Model {
                        id: s210_mirror_R_s210_block_0
                        objectName: "s210_mirror_R_s210_block_0"
                        source: node.assetUrl("meshes/s210_mirror_R_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_mirror_R_s210_sigR_0
                        objectName: "s210_mirror_R_s210_sigR_0"
                        source: node.assetUrl("meshes/s210_mirror_R_s210_sigR_0_mesh.mesh")
                        materials: [
                            s210_sigR_material
                        ]
                    }
                }
                Node {
                    id: s210_mirror_L
                    objectName: "s210_mirror_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_mirror_L_s210_body_0
                        objectName: "s210_mirror_L_s210_body_0"
                        source: node.assetUrl("meshes/s210_mirror_L_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                    Model {
                        id: s210_mirror_L_mirror_0
                        objectName: "s210_mirror_L_mirror_0"
                        source: node.assetUrl("meshes/s210_mirror_L_mirror_0_mesh.mesh")
                        materials: [
                            mirror_material
                        ]
                    }
                    Model {
                        id: s210_mirror_L_s210_block_0
                        objectName: "s210_mirror_L_s210_block_0"
                        source: node.assetUrl("meshes/s210_mirror_L_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_mirror_L_s210_sigL_0
                        objectName: "s210_mirror_L_s210_sigL_0"
                        source: node.assetUrl("meshes/s210_mirror_L_s210_sigL_0_mesh.mesh")
                        materials: [
                            s210_sigL_material
                        ]
                    }
                }
                Node {
                    id: s210_doorpanel_FL
                    objectName: "s210_doorpanel_FL"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorpanel_FL_etk_wheel_03a_0
                        objectName: "s210_doorpanel_FL_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_block_0
                        objectName: "s210_doorpanel_FL_s210_block_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_interior_LOD0_0
                        objectName: "s210_doorpanel_FL_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_leather_white_512_0
                        objectName: "s210_doorpanel_FL_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_leatherdark_0
                        objectName: "s210_doorpanel_FL_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_wood_057_diff_0
                        objectName: "s210_doorpanel_FL_s210_wood_057_diff_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_wood_057_diff_0_mesh.mesh")
                        materials: [
                            s210_wood_057_diff_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_stitches_0
                        objectName: "s210_doorpanel_FL_s210_stitches_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_stitches_0_mesh.mesh")
                        materials: [
                            s210_stitches_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FL_s210_noski_0
                        objectName: "s210_doorpanel_FL_s210_noski_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FL_s210_noski_0_mesh.mesh")
                        materials: [
                            s210_noski_material
                        ]
                    }
                }
                Node {
                    id: s210_doorpanel_FR
                    objectName: "s210_doorpanel_FR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorpanel_FR_etk_wheel_03a_0
                        objectName: "s210_doorpanel_FR_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_block_0
                        objectName: "s210_doorpanel_FR_s210_block_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_interior_LOD0_0
                        objectName: "s210_doorpanel_FR_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_leather_white_512_0
                        objectName: "s210_doorpanel_FR_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_leatherdark_0
                        objectName: "s210_doorpanel_FR_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_wood_057_diff_0
                        objectName: "s210_doorpanel_FR_s210_wood_057_diff_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_wood_057_diff_0_mesh.mesh")
                        materials: [
                            s210_wood_057_diff_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_stitches_0
                        objectName: "s210_doorpanel_FR_s210_stitches_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_stitches_0_mesh.mesh")
                        materials: [
                            s210_stitches_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_FR_s210_noski_0
                        objectName: "s210_doorpanel_FR_s210_noski_0"
                        source: node.assetUrl("meshes/s210_doorpanel_FR_s210_noski_0_mesh.mesh")
                        materials: [
                            s210_noski_material
                        ]
                    }
                }
                Node {
                    id: s210_doorpanel_RL
                    objectName: "s210_doorpanel_RL"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorpanel_RL_etk_wheel_03a_0
                        objectName: "s210_doorpanel_RL_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RL_s210_block_0
                        objectName: "s210_doorpanel_RL_s210_block_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RL_s210_interior_LOD0_0
                        objectName: "s210_doorpanel_RL_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RL_s210_leather_white_512_0
                        objectName: "s210_doorpanel_RL_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RL_s210_leatherdark_0
                        objectName: "s210_doorpanel_RL_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RL_s210_wood_057_diff_0
                        objectName: "s210_doorpanel_RL_s210_wood_057_diff_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_s210_wood_057_diff_0_mesh.mesh")
                        materials: [
                            s210_wood_057_diff_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RL_s210_stitches_0
                        objectName: "s210_doorpanel_RL_s210_stitches_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RL_s210_stitches_0_mesh.mesh")
                        materials: [
                            s210_stitches_material
                        ]
                    }
                }
                Node {
                    id: s210_doorpanel_RR
                    objectName: "s210_doorpanel_RR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorpanel_RR_etk_wheel_03a_0
                        objectName: "s210_doorpanel_RR_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RR_s210_block_0
                        objectName: "s210_doorpanel_RR_s210_block_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RR_s210_interior_LOD0_0
                        objectName: "s210_doorpanel_RR_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RR_s210_leather_white_512_0
                        objectName: "s210_doorpanel_RR_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RR_s210_leatherdark_0
                        objectName: "s210_doorpanel_RR_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RR_s210_wood_057_diff_0
                        objectName: "s210_doorpanel_RR_s210_wood_057_diff_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_s210_wood_057_diff_0_mesh.mesh")
                        materials: [
                            s210_wood_057_diff_material
                        ]
                    }
                    Model {
                        id: s210_doorpanel_RR_s210_stitches_0
                        objectName: "s210_doorpanel_RR_s210_stitches_0"
                        source: node.assetUrl("meshes/s210_doorpanel_RR_s210_stitches_0_mesh.mesh")
                        materials: [
                            s210_stitches_material
                        ]
                    }
                }
                Node {
                    id: s210_roof
                    objectName: "s210_roof"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_roof_s210_body_0
                        objectName: "s210_roof_s210_body_0"
                        source: node.assetUrl("meshes/s210_roof_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                }
                Node {
                    id: s210_headlightglass_L
                    objectName: "s210_headlightglass_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_headlightglass_L_s210_glass_0
                        objectName: "s210_headlightglass_L_s210_glass_0"
                        source: node.assetUrl("meshes/s210_headlightglass_L_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_headlight_R
                    objectName: "s210_headlight_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_headlight_R_wheel_42a_0
                        objectName: "s210_headlight_R_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_headlight_R_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_headlight_R_s210_block_0
                        objectName: "s210_headlight_R_s210_block_0"
                        source: node.assetUrl("meshes/s210_headlight_R_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_headlight_R_s210_sigL_0
                        objectName: "s210_headlight_R_s210_sigL_0"
                        source: node.assetUrl("meshes/s210_headlight_R_s210_sigL_0_mesh.mesh")
                        materials: [
                            s210_sigL_material
                        ]
                    }
                    Model {
                        id: s210_headlight_R_s210_headlight_0
                        objectName: "s210_headlight_R_s210_headlight_0"
                        source: node.assetUrl("meshes/s210_headlight_R_s210_headlight_0_mesh.mesh")
                        materials: [
                            s210_headlight_material
                        ]
                    }
                    Model {
                        id: s210_headlight_R_s210_runninglight_0
                        objectName: "s210_headlight_R_s210_runninglight_0"
                        source: node.assetUrl("meshes/s210_headlight_R_s210_runninglight_0_mesh.mesh")
                        materials: [
                            s210_runninglight_material
                        ]
                    }
                }
                Node {
                    id: s210_foglightglass_L_a
                    objectName: "s210_foglightglass_L_a"
                    y: 12.344419479370117
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_foglightglass_L_a_s210_glass_0
                        objectName: "s210_foglightglass_L_a_s210_glass_0"
                        source: node.assetUrl("meshes/s210_foglightglass_L_a_s210_glass_0_mesh.mesh")
                        materials: [
                            s210_glass_material
                        ]
                    }
                }
                Node {
                    id: s210_foglight_L_a
                    objectName: "s210_foglight_L_a"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_foglight_L_a_s210_fogl_0
                        objectName: "s210_foglight_L_a_s210_fogl_0"
                        source: node.assetUrl("meshes/s210_foglight_L_a_s210_fogl_0_mesh.mesh")
                        materials: [
                            s210_fogl_material
                        ]
                    }
                }
                Node {
                    id: s210_banka_stock_R
                    objectName: "s210_banka_stock_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_banka_stock_R_s210_0
                        objectName: "s210_banka_stock_R_s210_0"
                        source: node.assetUrl("meshes/s210_banka_stock_R_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_seat_FR
                    objectName: "s210_seat_FR"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_seat_FR_s210_leatherdark_0
                        objectName: "s210_seat_FR_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_seat_FR_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_seat_FR_s210_block_0
                        objectName: "s210_seat_FR_s210_block_0"
                        source: node.assetUrl("meshes/s210_seat_FR_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_seat_FR_s210_red_0
                        objectName: "s210_seat_FR_s210_red_0"
                        source: node.assetUrl("meshes/s210_seat_FR_s210_red_0_mesh.mesh")
                        materials: [
                            s210_red_material
                        ]
                    }
                }
                Node {
                    id: s210_seat_FL
                    objectName: "s210_seat_FL"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_seat_FL_s210_leatherdark_0
                        objectName: "s210_seat_FL_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_seat_FL_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_seat_FL_s210_block_0
                        objectName: "s210_seat_FL_s210_block_0"
                        source: node.assetUrl("meshes/s210_seat_FL_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_seat_FL_s210_red_0
                        objectName: "s210_seat_FL_s210_red_0"
                        source: node.assetUrl("meshes/s210_seat_FL_s210_red_0_mesh.mesh")
                        materials: [
                            s210_red_material
                        ]
                    }
                }
                Node {
                    id: s210_seats_R
                    objectName: "s210_seats_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_seats_R_s210_leatherdark_0
                        objectName: "s210_seats_R_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_seats_R_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_seats_R_s210_seatbelt_001_0
                        objectName: "s210_seats_R_s210_seatbelt_001_0"
                        source: node.assetUrl("meshes/s210_seats_R_s210_seatbelt_001_0_mesh.mesh")
                        materials: [
                            s210_seatbelt_001_material
                        ]
                    }
                }
                Node {
                    id: s210_intmirror
                    objectName: "s210_intmirror"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_intmirror_mirror_0
                        objectName: "s210_intmirror_mirror_0"
                        source: node.assetUrl("meshes/s210_intmirror_mirror_0_mesh.mesh")
                        materials: [
                            mirror_material
                        ]
                    }
                    Model {
                        id: s210_intmirror_s210_leatherdark_0
                        objectName: "s210_intmirror_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_intmirror_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                }
                Node {
                    id: s210_sunvisor
                    objectName: "s210_sunvisor"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_sunvisor_wheel_42a_0
                        objectName: "s210_sunvisor_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_sunvisor_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_sunvisor_s210_leather_white_512_0
                        objectName: "s210_sunvisor_s210_leather_white_512_0"
                        source: node.assetUrl("meshes/s210_sunvisor_s210_leather_white_512_0_mesh.mesh")
                        materials: [
                            s210_leather_white_512_material
                        ]
                    }
                    Model {
                        id: s210_sunvisor_s210_leatherdark_0
                        objectName: "s210_sunvisor_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_sunvisor_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                }
                Node {
                    id: s210_brakepedal
                    objectName: "s210_brakepedal"
                    x: -38.92987823486328
                    y: 51.220054626464844
                    z: 92.41876220703125
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_brakepedal_etk_wheel_03a_0
                        objectName: "s210_brakepedal_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_brakepedal_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_brakepedal_s210_interior_LOD0_0
                        objectName: "s210_brakepedal_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_brakepedal_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                }
                Node {
                    id: s210_gaspedal
                    objectName: "s210_gaspedal"
                    x: -22.53521156311035
                    y: 28.763690948486328
                    z: 77.93122863769531
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_gaspedal_etk_wheel_03a_0
                        objectName: "s210_gaspedal_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_gaspedal_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_gaspedal_s210_interior_LOD0_0
                        objectName: "s210_gaspedal_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_gaspedal_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                }
                Node {
                    id: s210_steer
                    objectName: "s210_steer"
                    x: -37.90475845336914
                    y: 93.4035415649414
                    z: 32.13165283203125
                    rotation: Qt.quaternion(-0.153044, 0.988219, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_steer_etk_wheel_03a_0
                        objectName: "s210_steer_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_steer_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_steer_s210_block_0
                        objectName: "s210_steer_s210_block_0"
                        source: node.assetUrl("meshes/s210_steer_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_steer_s210_leatherdark_0
                        objectName: "s210_steer_s210_leatherdark_0"
                        source: node.assetUrl("meshes/s210_steer_s210_leatherdark_0_mesh.mesh")
                        materials: [
                            s210_leatherdark_material
                        ]
                    }
                    Model {
                        id: s210_steer_s210_interior_LOD0_0
                        objectName: "s210_steer_s210_interior_LOD0_0"
                        source: node.assetUrl("meshes/s210_steer_s210_interior_LOD0_0_mesh.mesh")
                        materials: [
                            s210_interior_LOD0_material
                        ]
                    }
                    Model {
                        id: s210_steer_s210_noski_0
                        objectName: "s210_steer_s210_noski_0"
                        source: node.assetUrl("meshes/s210_steer_s210_noski_0_mesh.mesh")
                        materials: [
                            s210_noski_material
                        ]
                    }
                }
                Node {
                    id: s210_tubs
                    objectName: "s210_tubs"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_tubs_s210_0
                        objectName: "s210_tubs_s210_0"
                        source: node.assetUrl("meshes/s210_tubs_s210_0_mesh.mesh")
                        materials: [
                            s210_material
                        ]
                    }
                }
                Node {
                    id: s210_fender_L
                    objectName: "s210_fender_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_fender_L_s210_body_0
                        objectName: "s210_fender_L_s210_body_0"
                        source: node.assetUrl("meshes/s210_fender_L_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                }
                Node {
                    id: s210_fender_R
                    objectName: "s210_fender_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_fender_R_s210_body_0
                        objectName: "s210_fender_R_s210_body_0"
                        source: node.assetUrl("meshes/s210_fender_R_s210_body_0_mesh.mesh")
                        materials: [
                            s210_body_material
                        ]
                    }
                }
                Node {
                    id: s210_hybrid_L
                    objectName: "s210_hybrid_L"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_hybrid_L_wheel_42a_0
                        objectName: "s210_hybrid_L_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_hybrid_L_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_hybrid_L_s210_hyb_0
                        objectName: "s210_hybrid_L_s210_hyb_0"
                        source: node.assetUrl("meshes/s210_hybrid_L_s210_hyb_0_mesh.mesh")
                        materials: [
                            s210_hyb_material
                        ]
                    }
                }
                Node {
                    id: s210_hybrid_R
                    objectName: "s210_hybrid_R"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_hybrid_R_wheel_42a_0
                        objectName: "s210_hybrid_R_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_hybrid_R_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_hybrid_R_s210_hyb_0
                        objectName: "s210_hybrid_R_s210_hyb_0"
                        source: node.assetUrl("meshes/s210_hybrid_R_s210_hyb_0_mesh.mesh")
                        materials: [
                            s210_hyb_material
                        ]
                    }
                }
                Node {
                    id: s210_wipers
                    objectName: "s210_wipers"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_wipers_s210_block_0
                        objectName: "s210_wipers_s210_block_0"
                        source: node.assetUrl("meshes/s210_wipers_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_bumperbar_F
                    objectName: "s210_bumperbar_F"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_bumperbar_F_s210_block_0
                        objectName: "s210_bumperbar_F_s210_block_0"
                        source: node.assetUrl("meshes/s210_bumperbar_F_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_engbaycrap
                    objectName: "s210_engbaycrap"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_engbaycrap_wheel_42a_0
                        objectName: "s210_engbaycrap_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_engbaycrap_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_engbaycrap_s210_block_0
                        objectName: "s210_engbaycrap_s210_block_0"
                        source: node.assetUrl("meshes/s210_engbaycrap_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_RR_int
                    objectName: "s210_doorglass_RR_int"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_RR_int_s210_glass_int_0
                        objectName: "s210_doorglass_RR_int_s210_glass_int_0"
                        source: node.assetUrl("meshes/s210_doorglass_RR_int_s210_glass_int_0_mesh.mesh")
                        materials: [
                            s210_glass_int_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_FR_int
                    objectName: "s210_doorglass_FR_int"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_FR_int_s210_glass_int_0
                        objectName: "s210_doorglass_FR_int_s210_glass_int_0"
                        source: node.assetUrl("meshes/s210_doorglass_FR_int_s210_glass_int_0_mesh.mesh")
                        materials: [
                            s210_glass_int_material
                        ]
                    }
                }
                Node {
                    id: s210_doorglass_FL_int
                    objectName: "s210_doorglass_FL_int"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_doorglass_FL_int_s210_glass_int_0
                        objectName: "s210_doorglass_FL_int_s210_glass_int_0"
                        source: node.assetUrl("meshes/s210_doorglass_FL_int_s210_glass_int_0_mesh.mesh")
                        materials: [
                            s210_glass_int_material
                        ]
                    }
                }
                Node {
                    id: s210_lettering_athlete_s
                    objectName: "s210_lettering_athlete_s"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_lettering_athlete_s_wheel_42a_0
                        objectName: "s210_lettering_athlete_s_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_lettering_athlete_s_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                }
                Node {
                    id: s210_needle_tacho
                    objectName: "s210_needle_tacho"
                    x: -30.16476058959961
                    y: 94.37845611572266
                    z: 63.104408264160156
                    rotation: Qt.quaternion(-0.182235, 0.983255, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_needle_tacho_etk_wheel_03a_0
                        objectName: "s210_needle_tacho_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_needle_tacho_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_needle_tacho_s210_block_0
                        objectName: "s210_needle_tacho_s210_block_0"
                        source: node.assetUrl("meshes/s210_needle_tacho_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_needle_tacho_s210_numbers_0
                        objectName: "s210_needle_tacho_s210_numbers_0"
                        source: node.assetUrl("meshes/s210_needle_tacho_s210_numbers_0_mesh.mesh")
                        materials: [
                            s210_numbers_material
                        ]
                    }
                }
                Node {
                    id: s210_needle_speedo
                    objectName: "s210_needle_speedo"
                    x: -45.64990234375
                    y: 94.34791564941406
                    z: 63.451927185058594
                    rotation: Qt.quaternion(-0.182235, 0.983255, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_needle_speedo_etk_wheel_03a_0
                        objectName: "s210_needle_speedo_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_needle_speedo_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                    Model {
                        id: s210_needle_speedo_s210_block_0
                        objectName: "s210_needle_speedo_s210_block_0"
                        source: node.assetUrl("meshes/s210_needle_speedo_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_needle_speedo_s210_numbers_0
                        objectName: "s210_needle_speedo_s210_numbers_0"
                        source: node.assetUrl("meshes/s210_needle_speedo_s210_numbers_0_mesh.mesh")
                        materials: [
                            s210_numbers_material
                        ]
                    }
                }
                Node {
                    id: s210_intake_i6
                    objectName: "s210_intake_i6"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_intake_i6_s210_block_0
                        objectName: "s210_intake_i6_s210_block_0"
                        source: node.assetUrl("meshes/s210_intake_i6_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_header_i6
                    objectName: "s210_header_i6"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_header_i6_s210_block_0
                        objectName: "s210_header_i6_s210_block_0"
                        source: node.assetUrl("meshes/s210_header_i6_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                }
                Node {
                    id: s210_gauges
                    objectName: "s210_gauges"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_gauges_etk_wheel_03a_0
                        objectName: "s210_gauges_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_gauges_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: s210_gauges_screen
                    objectName: "s210_gauges_screen"
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_gauges_screen_etk_wheel_03a_0
                        objectName: "s210_gauges_screen_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_gauges_screen_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: object_4_003
                    objectName: "Object_4.003"
                    x: 75.43106842041016
                    y: 38.38960266113281
                    z: 140.20240783691406
                    rotation: Qt.quaternion(0.706434, -0.706434, -0.0308436, 0.0308436)
                    scale.x: 131.292
                    scale.y: 107.798
                    scale.z: 107.798
                    Model {
                        id: object_4_003_Scene___Root_002_0
                        objectName: "Object_4.003_Scene_-_Root.002_0"
                        source: node.assetUrl("meshes/object_4_003_Scene___Root_002_0_mesh.mesh")
                        materials: [
                            scene___Root_002_material
                        ]
                    }
                }
                Node {
                    id: amdb11_brakedisc_FR_003
                    objectName: "amdb11_brakedisc_FR.003"
                    x: 74.39662170410156
                    y: 38.29909896850586
                    z: 140.20240783691406
                    rotation: Qt.quaternion(0.0183679, -0.0183683, 0.706868, -0.706868)
                    scale.x: 116.446
                    scale.y: 99.9348
                    scale.z: 99.9348
                    Model {
                        id: amdb11_brakedisc_FR_003_amdb11_brake_002_0
                        objectName: "amdb11_brakedisc_FR.003_amdb11_brake.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_003_amdb11_brake_002_0_mesh.mesh")
                        materials: [
                            amdb11_brake_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_003_amdb11_misc_chrome_002_0
                        objectName: "amdb11_brakedisc_FR.003_amdb11_misc_chrome.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_003_amdb11_misc_chrome_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_chrome_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_003_amdb11_misc_002_0
                        objectName: "amdb11_brakedisc_FR.003_amdb11_misc.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_003_amdb11_misc_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_003_amdb11_badging_002_0
                        objectName: "amdb11_brakedisc_FR.003_amdb11_badging.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_003_amdb11_badging_002_0_mesh.mesh")
                        materials: [
                            amdb11_badging_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_003_amdb11_caliper_002_0
                        objectName: "amdb11_brakedisc_FR.003_amdb11_caliper.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_003_amdb11_caliper_002_0_mesh.mesh")
                        materials: [
                            amdb11_caliper_002_material
                        ]
                    }
                }
                Node {
                    id: s210_wheel_18x8_001
                    objectName: "s210_wheel_18x8.001"
                    x: 74.40277099609375
                    y: 38.29964065551758
                    z: -141.18222045898438
                    rotation: Qt.quaternion(0.706434, -0.706434, -0.0308436, 0.0308436)
                    scale.x: 122.624
                    scale.y: 122.624
                    scale.z: 122.624
                    Model {
                        id: s210_wheel_18x8_001_wheel_42a_0
                        objectName: "s210_wheel_18x8.001_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_001_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_001_s210_block_0
                        objectName: "s210_wheel_18x8.001_s210_block_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_001_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_001_etk_wheel_03a_0
                        objectName: "s210_wheel_18x8.001_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_001_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: object_4_001
                    objectName: "Object_4.001"
                    x: 75.43106842041016
                    y: 38.38960266113281
                    z: -141.18222045898438
                    rotation: Qt.quaternion(0.706434, -0.706434, -0.0308436, 0.0308436)
                    scale.x: 131.292
                    scale.y: 107.798
                    scale.z: 107.798
                    Model {
                        id: object_4_001_Scene___Root_002_0
                        objectName: "Object_4.001_Scene_-_Root.002_0"
                        source: node.assetUrl("meshes/object_4_001_Scene___Root_002_0_mesh.mesh")
                        materials: [
                            scene___Root_002_material
                        ]
                    }
                }
                Node {
                    id: amdb11_brakedisc_FR_001
                    objectName: "amdb11_brakedisc_FR.001"
                    x: 74.39662170410156
                    y: 38.29909896850586
                    z: -141.18222045898438
                    rotation: Qt.quaternion(0.0183679, -0.0183683, 0.706868, -0.706868)
                    scale.x: 116.446
                    scale.y: 99.9348
                    scale.z: 99.9348
                    Model {
                        id: amdb11_brakedisc_FR_001_amdb11_brake_002_0
                        objectName: "amdb11_brakedisc_FR.001_amdb11_brake.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_001_amdb11_brake_002_0_mesh.mesh")
                        materials: [
                            amdb11_brake_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_001_amdb11_misc_chrome_002_0
                        objectName: "amdb11_brakedisc_FR.001_amdb11_misc_chrome.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_001_amdb11_misc_chrome_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_chrome_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_001_amdb11_misc_002_0
                        objectName: "amdb11_brakedisc_FR.001_amdb11_misc.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_001_amdb11_misc_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_001_amdb11_badging_002_0
                        objectName: "amdb11_brakedisc_FR.001_amdb11_badging.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_001_amdb11_badging_002_0_mesh.mesh")
                        materials: [
                            amdb11_badging_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_001_amdb11_caliper_002_0
                        objectName: "amdb11_brakedisc_FR.001_amdb11_caliper.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_001_amdb11_caliper_002_0_mesh.mesh")
                        materials: [
                            amdb11_caliper_002_material
                        ]
                    }
                }
                Node {
                    id: s210_wheel_18x8_002
                    objectName: "s210_wheel_18x8.002"
                    x: -74.23978424072266
                    y: 38.29964065551758
                    z: 140.20240783691406
                    rotation: Qt.quaternion(0.706434, 0.706434, -0.0308436, -0.0308436)
                    scale.x: -122.624
                    scale.y: -122.624
                    scale.z: -122.624
                    Model {
                        id: s210_wheel_18x8_002_wheel_42a_0
                        objectName: "s210_wheel_18x8.002_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_002_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_002_s210_block_0
                        objectName: "s210_wheel_18x8.002_s210_block_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_002_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_002_etk_wheel_03a_0
                        objectName: "s210_wheel_18x8.002_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_002_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: object_4_002
                    objectName: "Object_4.002"
                    x: -75.26808166503906
                    y: 38.38960266113281
                    z: 140.20240783691406
                    rotation: Qt.quaternion(0.706434, 0.706434, -0.0308435, -0.0308436)
                    scale.x: -131.292
                    scale.y: -107.798
                    scale.z: -107.798
                    Model {
                        id: object_4_002_Scene___Root_002_0
                        objectName: "Object_4.002_Scene_-_Root.002_0"
                        source: node.assetUrl("meshes/object_4_002_Scene___Root_002_0_mesh.mesh")
                        materials: [
                            scene___Root_002_material
                        ]
                    }
                }
                Node {
                    id: amdb11_brakedisc_FR_002
                    objectName: "amdb11_brakedisc_FR.002"
                    x: -74.23363494873047
                    y: 38.29909896850586
                    z: 140.20240783691406
                    rotation: Qt.quaternion(0.0183683, 0.0183679, 0.706868, 0.706868)
                    scale.x: -116.446
                    scale.y: -99.9348
                    scale.z: -99.9348
                    Model {
                        id: amdb11_brakedisc_FR_002_amdb11_brake_002_0
                        objectName: "amdb11_brakedisc_FR.002_amdb11_brake.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_002_amdb11_brake_002_0_mesh.mesh")
                        materials: [
                            amdb11_brake_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_002_amdb11_misc_chrome_002_0
                        objectName: "amdb11_brakedisc_FR.002_amdb11_misc_chrome.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_002_amdb11_misc_chrome_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_chrome_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_002_amdb11_misc_002_0
                        objectName: "amdb11_brakedisc_FR.002_amdb11_misc.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_002_amdb11_misc_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_002_amdb11_badging_002_0
                        objectName: "amdb11_brakedisc_FR.002_amdb11_badging.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_002_amdb11_badging_002_0_mesh.mesh")
                        materials: [
                            amdb11_badging_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_002_amdb11_caliper_002_0
                        objectName: "amdb11_brakedisc_FR.002_amdb11_caliper.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_002_amdb11_caliper_002_0_mesh.mesh")
                        materials: [
                            amdb11_caliper_002_material
                        ]
                    }
                }
                Node {
                    id: s210_wheel_18x8_003
                    objectName: "s210_wheel_18x8.003"
                    x: -74.23978424072266
                    y: 38.29964065551758
                    z: -141.18222045898438
                    rotation: Qt.quaternion(0.706434, 0.706434, -0.0308436, -0.0308436)
                    scale.x: -122.624
                    scale.y: -122.624
                    scale.z: -122.624
                    Model {
                        id: s210_wheel_18x8_003_wheel_42a_0
                        objectName: "s210_wheel_18x8.003_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_003_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_003_s210_block_0
                        objectName: "s210_wheel_18x8.003_s210_block_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_003_s210_block_0_mesh.mesh")
                        materials: [
                            s210_block_material
                        ]
                    }
                    Model {
                        id: s210_wheel_18x8_003_etk_wheel_03a_0
                        objectName: "s210_wheel_18x8.003_etk_wheel_03a_0"
                        source: node.assetUrl("meshes/s210_wheel_18x8_003_etk_wheel_03a_0_mesh.mesh")
                        materials: [
                            etk_wheel_03a_material
                        ]
                    }
                }
                Node {
                    id: object_4_004
                    objectName: "Object_4.004"
                    x: -75.26808166503906
                    y: 38.38960266113281
                    z: -141.18222045898438
                    rotation: Qt.quaternion(0.706434, 0.706434, -0.0308435, -0.0308436)
                    scale.x: -131.292
                    scale.y: -107.798
                    scale.z: -107.798
                    Model {
                        id: object_4_004_Scene___Root_002_0
                        objectName: "Object_4.004_Scene_-_Root.002_0"
                        source: node.assetUrl("meshes/object_4_004_Scene___Root_002_0_mesh.mesh")
                        materials: [
                            scene___Root_002_material
                        ]
                    }
                }
                Node {
                    id: amdb11_brakedisc_FR_004
                    objectName: "amdb11_brakedisc_FR.004"
                    x: -74.23363494873047
                    y: 38.29909896850586
                    z: -141.18222045898438
                    rotation: Qt.quaternion(0.0183683, 0.0183679, 0.706868, 0.706868)
                    scale.x: -116.446
                    scale.y: -99.9348
                    scale.z: -99.9348
                    Model {
                        id: amdb11_brakedisc_FR_004_amdb11_brake_002_0
                        objectName: "amdb11_brakedisc_FR.004_amdb11_brake.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_004_amdb11_brake_002_0_mesh.mesh")
                        materials: [
                            amdb11_brake_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_004_amdb11_misc_chrome_002_0
                        objectName: "amdb11_brakedisc_FR.004_amdb11_misc_chrome.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_004_amdb11_misc_chrome_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_chrome_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_004_amdb11_misc_002_0
                        objectName: "amdb11_brakedisc_FR.004_amdb11_misc.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_004_amdb11_misc_002_0_mesh.mesh")
                        materials: [
                            amdb11_misc_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_004_amdb11_badging_002_0
                        objectName: "amdb11_brakedisc_FR.004_amdb11_badging.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_004_amdb11_badging_002_0_mesh.mesh")
                        materials: [
                            amdb11_badging_002_material
                        ]
                    }
                    Model {
                        id: amdb11_brakedisc_FR_004_amdb11_caliper_002_0
                        objectName: "amdb11_brakedisc_FR.004_amdb11_caliper.002_0"
                        source: node.assetUrl("meshes/amdb11_brakedisc_FR_004_amdb11_caliper_002_0_mesh.mesh")
                        materials: [
                            amdb11_caliper_002_material
                        ]
                    }
                }
                Node {
                    id: s210_grille_001
                    objectName: "s210_grille.001"
                    z: 0.7991452813148499
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale.x: 100
                    scale.y: 100
                    scale.z: 100
                    Model {
                        id: s210_grille_001_wheel_42a_0
                        objectName: "s210_grille.001_wheel_42a_0"
                        source: node.assetUrl("meshes/s210_grille_001_wheel_42a_0_mesh.mesh")
                        materials: [
                            wheel_42a_material
                        ]
                    }
                }
            }
        }
    }

    Node {
        id: __materialLibrary__

    Texture {
        id: _0_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData9
        objectName: "_0_texture"
    }

    Texture {
        id: _1_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData11
        objectName: "_1_texture"
    }

    Texture {
        id: _5_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData299
        objectName: "_5_texture"
    }

    Texture {
        id: _2_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData16
        objectName: "_2_texture"
    }

    Texture {
        id: _3_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData18
        objectName: "_3_texture"
    }

    Texture {
        id: _6_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData301
        objectName: "_6_texture"
    }

    Texture {
        id: _11_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData346
        objectName: "_11_texture"
    }

    Texture {
        id: _7_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData
        objectName: "_7_texture"
    }

    Texture {
        id: _10_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData333
        objectName: "_10_texture"
    }

    Texture {
        id: _13_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData451
        objectName: "_13_texture"
    }

    Texture {
        id: _4_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData294
        objectName: "_4_texture"
    }

    Texture {
        id: _14_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData453
        objectName: "_14_texture"
    }

    Texture {
        id: _8_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData311
        objectName: "_8_texture"
    }

    Texture {
        id: _15_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData509
        objectName: "_15_texture"
    }

    Texture {
        id: _12_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData449
        objectName: "_12_texture"
    }

    Texture {
        id: _16_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData511
        objectName: "_16_texture"
    }

    Texture {
        id: _9_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData313
        objectName: "_9_texture"
    }

    PrincipledMaterial {
        id: s210_glass_material
        objectName: "s210_glass"
        baseColor: "#650a0a0a"
        metalness: 0.8091453313827515
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Blend
    }

    PrincipledMaterial {
        id: s210_lights_glass_material
        objectName: "s210_lights_glass"
        baseColor: "#9b1a1a1a"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Blend
    }

    PrincipledMaterial {
        id: s210_block_material
        objectName: "s210_block"
        baseColor: "#ff0c0c0c"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: etk_wheel_03a_material
        objectName: "etk_wheel_03a"
        baseColor: "#ffe6e6e6"
        metalness: 1
        roughness: 0.3005692958831787
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_body_material
        objectName: "s210_body"
        baseColor: "#ffb9b9b9"
        metalness: 0.5104777216911316
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
        clearcoatAmount: 1
        clearcoatRoughnessAmount: 0.03999999910593033
    }

    PrincipledMaterial {
        id: wheel_42a_material
        objectName: "wheel_42a"
        baseColor: "#ff999999"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: bastion_engine_material
        objectName: "bastion_engine"
        baseColor: "#ff070707"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_material
        objectName: "s210"
        baseColor: "#ff000000"
        metalness: 0.9737172722816467
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: etk_engine_i6_material
        objectName: "etk_engine_i6"
        baseColor: "#ff969696"
        roughness: 0.8685191869735718
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_interior_LOD0_material
        objectName: "s210_interior_LOD0"
        baseColor: "#ffcccccc"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_leather_white_512_material
        objectName: "s210_leather_white_512"
        baseColor: "#ffa1816f"
        metalness: 1
        roughness: 0.8821027874946594
        normalMap: _4_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_carpet_material
        objectName: "s210_carpet"
        baseColorMap: _5_texture
        metalness: 0.6445733904838562
        roughness: 0.8211145401000977
        normalMap: _6_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_leatherdark_material
        objectName: "s210_leatherdark"
        baseColor: "#ff0f0f0f"
        metalness: 0.7177164554595947
        roughness: 0.98504638671875
        normalMap: _7_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_seatbelt_001_material
        objectName: "s210_seatbelt_001"
        baseColorMap: _8_texture
        roughness: 0.8211145401000977
        normalMap: _9_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_sperm_material
        objectName: "s210_sperm"
        baseColor: "#ffcccccc"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_wood_057_diff_material
        objectName: "s210_wood_057_diff"
        baseColor: "#ff0e0e0e"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_dd_material
        objectName: "s210_dd"
        baseColorMap: _10_texture
        metalness: 0.8579073548316956
        roughness: 0.044568516314029694
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_screen_material
        objectName: "s210_screen"
        baseColorMap: _10_texture
        metalness: 0.985907793045044
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_numbers_material
        objectName: "s210_numbers"
        baseColor: "#ff5459cc"
        roughness: 0.9158447980880737
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_comp_material
        objectName: "s210_comp"
        baseColorMap: _11_texture
        roughness: 0.8211145401000977
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_noski_material
        objectName: "s210_noski"
        baseColor: "#00cccccc"
        roughness: 0.8211145401000977
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Blend
    }

    PrincipledMaterial {
        id: s210_fogl_material
        objectName: "s210_fogl"
        baseColor: "#ffcccccc"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_runninglight_material
        objectName: "s210_runninglight"
        baseColor: "#ffadc6ca"
        metalness: 0.6811449527740479
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_sigL_material
        objectName: "s210_sigL"
        baseColor: "#ff6d3f32"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_headlight_material
        objectName: "s210_headlight"
        baseColor: "#ff4e5d96"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_hyb_material
        objectName: "s210_hyb"
        baseColorMap: _12_texture
        metalnessMap: _13_texture
        roughnessMap: _13_texture
        metalness: 1
        normalMap: _14_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_revik_material
        objectName: "s210_revik"
        baseColorMap: _0_texture
        metalness: 1
        normalMap: _1_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_lights1_material
        objectName: "s210_lights1"
        baseColorMap: _2_texture
        metalnessMap: _3_texture
        roughnessMap: _3_texture
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_lowhi_material
        objectName: "s210_lowhi"
        baseColor: "#ff410001"
        roughness: 0.9852643013000488
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: mirror_material
        objectName: "mirror"
        baseColor: "#ff969696"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_sigR_material
        objectName: "s210_sigR"
        baseColor: "#ff767676"
        roughness: 0.8778607845306396
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_stitches_material
        objectName: "s210_stitches"
        baseColorMap: _15_texture
        roughness: 0.8211145401000977
        normalMap: _16_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Blend
    }

    PrincipledMaterial {
        id: s210_red_material
        objectName: "s210_red"
        baseColor: "#ffcccccc"
        roughness: 0.8211145401000977
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: s210_glass_int_material
        objectName: "s210_glass_int"
        baseColor: "#00000000"
        roughness: 0.9996068477630615
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Blend
    }

    PrincipledMaterial {
        id: scene___Root_002_material
        objectName: "Scene_-_Root.002"
        baseColor: "#ff000000"
        metalness: 0.6506686210632324
        roughness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: amdb11_brake_002_material
        objectName: "amdb11_brake.002"
        baseColor: "#ff1e1e1e"
        metalness: 0.5287635326385498
        roughness: 0.3615218698978424
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: amdb11_misc_chrome_002_material
        objectName: "amdb11_misc_chrome.002"
        baseColor: "#ffbcbcbc"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: amdb11_misc_002_material
        objectName: "amdb11_misc.002"
        baseColor: "#ff898989"
        metalness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: amdb11_badging_002_material
        objectName: "amdb11_badging.002"
        baseColor: "#ff000000"
        roughness: 1
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: amdb11_caliper_002_material
        objectName: "amdb11_caliper.002"
        baseColor: "#ff353535"
        metalness: 1
        roughness: 0.3554266095161438
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    }

    // DirectionalLight {
    //     id: keyLight
    //     brightness: node.highQuality ? 3.5 : 5.0
    //     castsShadow: false
    //     eulerRotation.x: -38
    //     eulerRotation.y: 34
    //     eulerRotation.z: 0
    // }

    // DirectionalLight {
    //     id: fillLight
    //     brightness: node.highQuality ? 1.2 : 2.2
    //     castsShadow: false
    //     eulerRotation.x: -18
    //     eulerRotation.y: -145
    //     eulerRotation.z: 0
    // }

    // DirectionalLight {
    //     id: rimLight
    //     brightness: node.highQuality ? 0.8 : 1.4
    //     castsShadow: false
    //     eulerRotation.x: 18
    //     eulerRotation.y: 180
    //     eulerRotation.z: 0
    // }

    PerspectiveCamera {
        id: perspectiveCamera
        eulerRotation.y: 27.64407
        x: 2.515
        y: 1.3
        eulerRotation.z: -0.00002
        eulerRotation.x: 0
        z: 5.55252
    }

    PerspectiveCamera {
        id: perspectiveCamera1
        x: -3
        y: 2
        z: -6
        eulerRotation.y: 213.89067
    }

    // Animations:
}

/*##^##
Designer {
    D{i:0;cameraSpeed3d:10;cameraSpeed3dMultiplier:1}
}
##^##*/
