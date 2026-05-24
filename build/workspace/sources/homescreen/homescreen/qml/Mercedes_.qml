import QtQuick
import QtQuick3D
import QtQuick.Timeline

Node {
    id: node

    property alias rootNode: rootNode
    property alias camera: perspectiveCamera1

    // Resources
    property url textureData: "image://shared/textureData"
    property url textureData302: "image://shared/textureData302"
    property url textureData104: "image://shared/textureData104"
    property url textureData81: "image://shared/textureData81"
    property url textureData106: "image://shared/textureData106"
    property url textureData90: "image://shared/textureData90"
    property url textureData86: "image://shared/textureData86"
    property url textureData108: "image://shared/textureData108"
    property url textureData50: "image://shared/textureData50"
    property url textureData95: "image://shared/textureData95"
    property url textureData88: "image://shared/textureData88"
    property url textureData41: "image://shared/textureData41"
    property url textureData193: "image://shared/textureData193"
    property url textureData43: "image://shared/textureData43"
    property url textureData195: "image://shared/textureData195"
    property url textureData97: "image://shared/textureData97"
    property url textureData197: "image://shared/textureData197"
    property url textureData48: "image://shared/textureData48"
    property url textureData202: "image://shared/textureData202"
    property url textureData206: "image://shared/textureData206"
    property url textureData204: "image://shared/textureData204"
    property url textureData55: "image://shared/textureData55"
    property url textureData57: "image://shared/textureData57"
    property url textureData59: "image://shared/textureData59"
    property url textureData99: "image://shared/textureData99"
    property url textureData64: "image://shared/textureData64"
    property url textureData77: "image://shared/textureData77"
    property url textureData66: "image://shared/textureData66"
    property url textureData281: "image://shared/textureData281"
    property url textureData68: "image://shared/textureData68"
    property url textureData113: "image://shared/textureData113"
    property url textureData283: "image://shared/textureData283"
    property url textureData285: "image://shared/textureData285"
    property url textureData73: "image://shared/textureData73"
    property url textureData304: "image://shared/textureData304"
    property url textureData75: "image://shared/textureData75"
    property url textureData306: "image://shared/textureData306"

    // Nodes:
    Node {
        id: sketchfab_model
        scale.z: 30.36344
        scale.y: 30.36344
        scale.x: 30.36344
        objectName: "Sketchfab_model"
        rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
        Node {
            id: fbx_merge_fbx
            objectName: "fbx_merge.fbx"
            rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
            Node {
                id: object_2
                objectName: "Object_2"
                Node {
                    id: rootNode
                    objectName: "RootNode"
                    Node {
                        id: root
                        objectName: "Root"
                        rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                        scale.x: 1
                        scale.y: 1
                        scale.z: 1
                        Node {
                            id: body
                            objectName: "Body"
                            y: -8.418671093579633e-09
                            z: 0.19678828120231628
                            Node {
                                id: doorFL
                                objectName: "DoorFL"
                                x: -0.9097139835357666
                                y: 0.8886669874191284
                                z: 0.5428969860076904
                                Node {
                                    id: mesh_DoorFL
                                    objectName: "mesh_DoorFL"
                                    Node {
                                        id: object_24
                                        objectName: "Object_24"
                                        x: 0.9097137451171875
                                        y: -1.1569385528564453
                                        z: -0.02989959716796875
                                        Model {
                                            id: mesh_DoorFL_mat_rubber_0
                                            objectName: "mesh_DoorFL_mat_rubber_0"
                                            source: "mesh_DoorFL_mat_rubber_0_mesh.mesh"
                                            materials: [
                                                mat_rubber_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_glass_0
                                            objectName: "mesh_DoorFL_mat_glass_0"
                                            source: "mesh_DoorFL_mat_glass_0_mesh.mesh"
                                            materials: [
                                                mat_glass_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_plastic_0
                                            objectName: "mesh_DoorFL_mat_plastic_0"
                                            source: "mesh_DoorFL_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_chrome_0
                                            objectName: "mesh_DoorFL_mat_chrome_0"
                                            source: "mesh_DoorFL_mat_chrome_0_mesh.mesh"
                                            materials: [
                                                mat_chrome_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_lights_0
                                            objectName: "mesh_DoorFL_mat_lights_0"
                                            source: "mesh_DoorFL_mat_lights_0_mesh.mesh"
                                            materials: [
                                                mat_lights_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_leather_0
                                            objectName: "mesh_DoorFL_mat_leather_0"
                                            source: "mesh_DoorFL_mat_leather_0_mesh.mesh"
                                            materials: [
                                                mat_leather_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_inside_panel_0
                                            objectName: "mesh_DoorFL_mat_inside_panel_0"
                                            source: "mesh_DoorFL_mat_inside_panel_0_mesh.mesh"
                                            materials: [
                                                mat_inside_panel_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_salon_0
                                            objectName: "mesh_DoorFL_mat_salon_0"
                                            source: "mesh_DoorFL_mat_salon_0_mesh.mesh"
                                            materials: [
                                                mat_salon_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFL_mat_body_0
                                            objectName: "mesh_DoorFL_mat_body_0"
                                            source: "mesh_DoorFL_mat_body_0_mesh.mesh"
                                            materials: mat_body_material
                                        }
                                    }
                                }
                            }
                            Node {
                                id: doorFR
                                objectName: "DoorFR"
                                x: 0.9097139835357666
                                y: 0.8886669874191284
                                z: 0.5428969860076904
                                Node {
                                    id: mesh_DoorFR
                                    objectName: "mesh_DoorFR"
                                    Node {
                                        id: object_36
                                        objectName: "Object_36"
                                        x: -0.9097139835357666
                                        y: -1.1569385528564453
                                        z: -0.02989959716796875
                                        Model {
                                            id: mesh_DoorFR_mat_body_0
                                            objectName: "mesh_DoorFR_mat_body_0"
                                            source: "mesh_DoorFR_mat_body_0_mesh.mesh"
                                            materials: mat_body_material
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_rubber_0
                                            objectName: "mesh_DoorFR_mat_rubber_0"
                                            source: "mesh_DoorFR_mat_rubber_0_mesh.mesh"
                                            materials: [
                                                mat_rubber_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_glass_0
                                            objectName: "mesh_DoorFR_mat_glass_0"
                                            source: "mesh_DoorFR_mat_glass_0_mesh.mesh"
                                            materials: [
                                                mat_glass_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_plastic_0
                                            objectName: "mesh_DoorFR_mat_plastic_0"
                                            source: "mesh_DoorFR_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_chrome_0
                                            objectName: "mesh_DoorFR_mat_chrome_0"
                                            source: "mesh_DoorFR_mat_chrome_0_mesh.mesh"
                                            materials: [
                                                mat_chrome_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_lights_0
                                            objectName: "mesh_DoorFR_mat_lights_0"
                                            source: "mesh_DoorFR_mat_lights_0_mesh.mesh"
                                            materials: [
                                                mat_lights_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_leather_0
                                            objectName: "mesh_DoorFR_mat_leather_0"
                                            source: "mesh_DoorFR_mat_leather_0_mesh.mesh"
                                            materials: [
                                                mat_leather_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_inside_panel_0
                                            objectName: "mesh_DoorFR_mat_inside_panel_0"
                                            source: "mesh_DoorFR_mat_inside_panel_0_mesh.mesh"
                                            materials: [
                                                mat_inside_panel_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorFR_mat_salon_0
                                            objectName: "mesh_DoorFR_mat_salon_0"
                                            source: "mesh_DoorFR_mat_salon_0_mesh.mesh"
                                            materials: [
                                                mat_salon_material
                                            ]
                                        }
                                    }
                                }
                            }
                            Node {
                                id: doorRL
                                objectName: "DoorRL"
                                x: -0.8970869779586792
                                y: -0.24632999300956726
                                z: 0.34468701481819153
                                Node {
                                    id: mesh_DoorRL
                                    objectName: "mesh_DoorRL"
                                    Node {
                                        id: object_48
                                        objectName: "Object_48"
                                        x: 0.8970870971679688
                                        y: -0.021942138671875
                                        z: 0.16831016540527344
                                        Model {
                                            id: mesh_DoorRL_mat_rubber_0
                                            objectName: "mesh_DoorRL_mat_rubber_0"
                                            source: "mesh_DoorRL_mat_rubber_0_mesh.mesh"
                                            materials: [
                                                mat_rubber_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_glass_0
                                            objectName: "mesh_DoorRL_mat_glass_0"
                                            source: "mesh_DoorRL_mat_glass_0_mesh.mesh"
                                            materials: [
                                                mat_glass_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_plastic_0
                                            objectName: "mesh_DoorRL_mat_plastic_0"
                                            source: "mesh_DoorRL_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_chrome_0
                                            objectName: "mesh_DoorRL_mat_chrome_0"
                                            source: "mesh_DoorRL_mat_chrome_0_mesh.mesh"
                                            materials: [
                                                mat_chrome_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_leather_0
                                            objectName: "mesh_DoorRL_mat_leather_0"
                                            source: "mesh_DoorRL_mat_leather_0_mesh.mesh"
                                            materials: [
                                                mat_leather_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_inside_panel_0
                                            objectName: "mesh_DoorRL_mat_inside_panel_0"
                                            source: "mesh_DoorRL_mat_inside_panel_0_mesh.mesh"
                                            materials: [
                                                mat_inside_panel_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_salon_0
                                            objectName: "mesh_DoorRL_mat_salon_0"
                                            source: "mesh_DoorRL_mat_salon_0_mesh.mesh"
                                            materials: [
                                                mat_salon_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRL_mat_body_0
                                            objectName: "mesh_DoorRL_mat_body_0"
                                            source: "mesh_DoorRL_mat_body_0_mesh.mesh"
                                            materials: mat_body_material
                                        }
                                    }
                                }
                            }
                            Node {
                                id: doorRR
                                objectName: "DoorRR"
                                x: 0.8970869779586792
                                y: -0.24632999300956726
                                z: 0.34468701481819153
                                Node {
                                    id: mesh_DoorRR
                                    objectName: "mesh_DoorRR"
                                    Node {
                                        id: object_59
                                        objectName: "Object_59"
                                        x: -0.8970869779586792
                                        y: -0.021942138671875
                                        z: 0.16831016540527344
                                        Model {
                                            id: mesh_DoorRR_mat_body_0
                                            objectName: "mesh_DoorRR_mat_body_0"
                                            source: "mesh_DoorRR_mat_body_0_mesh.mesh"
                                            materials: mat_body_material
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_rubber_0
                                            objectName: "mesh_DoorRR_mat_rubber_0"
                                            source: "mesh_DoorRR_mat_rubber_0_mesh.mesh"
                                            materials: [
                                                mat_rubber_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_glass_0
                                            objectName: "mesh_DoorRR_mat_glass_0"
                                            source: "mesh_DoorRR_mat_glass_0_mesh.mesh"
                                            materials: [
                                                mat_glass_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_plastic_0
                                            objectName: "mesh_DoorRR_mat_plastic_0"
                                            source: "mesh_DoorRR_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_chrome_0
                                            objectName: "mesh_DoorRR_mat_chrome_0"
                                            source: "mesh_DoorRR_mat_chrome_0_mesh.mesh"
                                            materials: [
                                                mat_chrome_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_leather_0
                                            objectName: "mesh_DoorRR_mat_leather_0"
                                            source: "mesh_DoorRR_mat_leather_0_mesh.mesh"
                                            materials: [
                                                mat_leather_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_inside_panel_0
                                            objectName: "mesh_DoorRR_mat_inside_panel_0"
                                            source: "mesh_DoorRR_mat_inside_panel_0_mesh.mesh"
                                            materials: [
                                                mat_inside_panel_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_DoorRR_mat_salon_0
                                            objectName: "mesh_DoorRR_mat_salon_0"
                                            source: "mesh_DoorRR_mat_salon_0_mesh.mesh"
                                            materials: [
                                                mat_salon_material
                                            ]
                                        }
                                    }
                                }
                            }
                            Node {
                                id: hood
                                objectName: "Hood"
                                y: 0.8496159911155701
                                z: 0.7392359972000122
                                Node {
                                    id: mesh_Hood
                                    objectName: "mesh_Hood"
                                    Node {
                                        id: object_70
                                        objectName: "Object_70"
                                        x: -5.960464477539063e-08
                                        y: -1.1178874969482422
                                        z: -0.22623825073242188
                                        Model {
                                            id: mesh_Hood_mat_body_0
                                            objectName: "mesh_Hood_mat_body_0"
                                            source: "mesh_Hood_mat_body_0_mesh.mesh"
                                            materials: mat_body_material
                                        }
                                        Model {
                                            id: mesh_Hood_mat_plastic_0
                                            objectName: "mesh_Hood_mat_plastic_0"
                                            source: "mesh_Hood_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_Hood_mat_chrome_0
                                            objectName: "mesh_Hood_mat_chrome_0"
                                            source: "mesh_Hood_mat_chrome_0_mesh.mesh"
                                            materials: [
                                                mat_chrome_material
                                            ]
                                        }
                                    }
                                }
                            }
                            Node {
                                id: trunk
                                objectName: "Trunk"
                                y: -1.8027700185775757
                                z: 0.8263980150222778
                                Node {
                                    id: mesh_Trunk
                                    objectName: "mesh_Trunk"
                                    Node {
                                        id: object_76
                                        objectName: "Object_76"
                                        y: 1.5344982147216797
                                        z: -0.3134002685546875
                                        Model {
                                            id: mesh_Trunk_mat_plastic_0
                                            objectName: "mesh_Trunk_mat_plastic_0"
                                            source: "mesh_Trunk_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_Trunk_mat_chrome_0
                                            objectName: "mesh_Trunk_mat_chrome_0"
                                            source: "mesh_Trunk_mat_chrome_0_mesh.mesh"
                                            materials: [
                                                mat_chrome_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_Trunk_mat_lights_glass_0
                                            objectName: "mesh_Trunk_mat_lights_glass_0"
                                            source: "mesh_Trunk_mat_lights_glass_0_mesh.mesh"
                                            materials: [
                                                mat_lights_glass_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_Trunk_mat_lights_glass2_0
                                            objectName: "mesh_Trunk_mat_lights_glass2_0"
                                            source: "mesh_Trunk_mat_lights_glass2_0_mesh.mesh"
                                            materials: [
                                                mat_lights_glass2_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_Trunk_mat_lights_0
                                            objectName: "mesh_Trunk_mat_lights_0"
                                            source: "mesh_Trunk_mat_lights_0_mesh.mesh"
                                            materials: [
                                                mat_lights_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_Trunk_mat_body_0
                                            objectName: "mesh_Trunk_mat_body_0"
                                            source: "mesh_Trunk_mat_body_0_mesh.mesh"
                                            materials: mat_body_material
                                        }
                                    }
                                }
                            }
                            Node {
                                id: wipper1
                                objectName: "Wipper1"
                                x: -0.6661345958709717
                                y: 0.9278092384338379
                                z: 0.7201446294784546
                                rotation: Qt.quaternion(0.968582, -0.243324, -0.0222225, 0.0463549)
                                Node {
                                    id: mesh_Wipper1
                                    objectName: "mesh_Wipper1"
                                    y: 2.9802322387695312e-08
                                    z: -2.220446049250313e-16
                                    rotation: Qt.quaternion(1, 1.49012e-08, 9.31322e-09, 6.98492e-09)
                                    Node {
                                        id: object_105
                                        objectName: "Object_105"
                                        x: 0.5380299091339111
                                        y: -1.003854751586914
                                        z: -0.787434458732605
                                        rotation: Qt.quaternion(0.968582, 0.243324, 0.0222225, -0.046355)
                                        Model {
                                            id: mesh_Wipper1_mat_plastic_0
                                            objectName: "mesh_Wipper1_mat_plastic_0"
                                            source: "mesh_Wipper1_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                    }
                                }
                            }
                            Node {
                                id: wipper2
                                objectName: "Wipper2"
                                x: 0.031309425830841064
                                y: 0.9950413703918457
                                z: 0.7783725261688232
                                rotation: Qt.quaternion(0.989138, -0.140995, 0.0220491, -0.0352121)
                                scale.x: 1
                                scale.y: 1
                                scale.z: 1
                                Node {
                                    id: wipper2_1
                                    objectName: "Wipper2_1"
                                    x: 0.03511402755975723
                                    y: 0.11254119873046875
                                    z: -0.016107797622680664
                                    rotation: Qt.quaternion(0.997387, -0.07225, -5.23689e-09, -5.04817e-09)
                                    Node {
                                        id: mesh_Wipper2_1
                                        objectName: "mesh_Wipper2_1"
                                        rotation: Qt.quaternion(1, 1.49012e-08, 7.45058e-09, -4.19095e-09)
                                        Node {
                                            id: object_110
                                            objectName: "Object_110"
                                            x: 0.03848126158118248
                                            y: -1.151885747909546
                                            z: -0.7645402550697327
                                            rotation: Qt.quaternion(0.976366, 0.212092, -0.0245355, 0.033527)
                                            scale.x: 1
                                            scale.y: 1
                                            scale.z: 1
                                            Model {
                                                id: mesh_Wipper2_1_mat_plastic_0
                                                objectName: "mesh_Wipper2_1_mat_plastic_0"
                                                source: "mesh_Wipper2_1_mat_plastic_0_mesh.mesh"
                                                materials: [
                                                    mat_plastic_material
                                                ]
                                            }
                                        }
                                    }
                                }
                                Node {
                                    id: wipper2_2
                                    objectName: "Wipper2_2"
                                    x: 0.1979944258928299
                                    y: 0.08520889282226562
                                    z: -0.0038514137268066406
                                    rotation: Qt.quaternion(0.997869, 0.00201731, 0.0530481, 0.0379445)
                                    scale.x: 1
                                    scale.y: 1
                                    scale.z: 1
                                    Node {
                                        id: mesh_Wipper2_2
                                        objectName: "mesh_Wipper2_2"
                                        x: -2.7755575615628914e-17
                                        y: 1.1920928955078125e-07
                                        rotation: Qt.quaternion(1, -2.23517e-07, -2.98023e-08, 1.86264e-09)
                                        Node {
                                            id: object_114
                                            objectName: "Object_114"
                                            x: -0.15246883034706116
                                            y: -1.2146433591842651
                                            z: -0.6125936508178711
                                            rotation: Qt.quaternion(0.987481, 0.135995, -0.0797531, 0.00512871)
                                            Model {
                                                id: mesh_Wipper2_2_mat_plastic_0
                                                objectName: "mesh_Wipper2_2_mat_plastic_0"
                                                source: "mesh_Wipper2_2_mat_plastic_0_mesh.mesh"
                                                materials: [
                                                    mat_plastic_material
                                                ]
                                            }
                                        }
                                    }
                                }
                                Node {
                                    id: mesh_Wipper2
                                    objectName: "mesh_Wipper2"
                                    rotation: Qt.quaternion(1, -2.08616e-07, 3.72529e-09, -9.31323e-09)
                                    Node {
                                        id: object_117
                                        objectName: "Object_117"
                                        x: 0.0735953152179718
                                        y: -1.1375066041946411
                                        z: -0.6066540479660034
                                        rotation: Qt.quaternion(0.989138, 0.140995, -0.0220492, 0.0352121)
                                        Model {
                                            id: mesh_Wipper2_mat_plastic_0
                                            objectName: "mesh_Wipper2_mat_plastic_0"
                                            source: "mesh_Wipper2_mat_plastic_0_mesh.mesh"
                                            materials: [
                                                mat_plastic_material
                                            ]
                                        }
                                    }
                                }
                            }
                            Node {
                                id: mesh_Body
                                objectName: "mesh_Body"
                                Node {
                                    id: object_120
                                    objectName: "Object_120"
                                    y: -0.26827192306518555
                                    z: 0.5129973888397217

                                    // Model {
                                    //     id: mesh_Body_mat_body_0
                                    //     objectName: "mesh_Body_mat_body_0"
                                    //     source: "mesh_Body_mat_body_0_mesh.mesh"
                                    //     materials: mat_body_material
                                    // }
                                    Model {
                                        id: mesh_Body_mat_bottom_0
                                        objectName: "mesh_Body_mat_bottom_0"
                                        source: "mesh_Body_mat_bottom_0_mesh.mesh"
                                        materials: [
                                            mat_bottom_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_glass_0
                                        objectName: "mesh_Body_mat_glass_0"
                                        source: "mesh_Body_mat_glass_0_mesh.mesh"
                                        materials: [
                                            mat_glass_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_plastic_0
                                        objectName: "mesh_Body_mat_plastic_0"
                                        source: "mesh_Body_mat_plastic_0_mesh.mesh"
                                        materials: [
                                            mat_plastic_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_chrome_0
                                        objectName: "mesh_Body_mat_chrome_0"
                                        source: "mesh_Body_mat_chrome_0_mesh.mesh"
                                        materials: [
                                            mat_chrome_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_lights_glass_0
                                        objectName: "mesh_Body_mat_lights_glass_0"
                                        source: "mesh_Body_mat_lights_glass_0_mesh.mesh"
                                        materials: [
                                            mat_lights_glass_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_lights_glass2_0
                                        objectName: "mesh_Body_mat_lights_glass2_0"
                                        source: "mesh_Body_mat_lights_glass2_0_mesh.mesh"
                                        materials: [
                                            mat_lights_glass2_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_lights_0
                                        objectName: "mesh_Body_mat_lights_0"
                                        source: "mesh_Body_mat_lights_0_mesh.mesh"
                                        materials: [
                                            mat_lights_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_inside_gauges_0
                                        objectName: "mesh_Body_mat_inside_gauges_0"
                                        source: "mesh_Body_mat_inside_gauges_0_mesh.mesh"
                                        materials: [
                                            mat_inside_gauges_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_leather_0
                                        objectName: "mesh_Body_mat_leather_0"
                                        source: "mesh_Body_mat_leather_0_mesh.mesh"
                                        materials: [
                                            mat_leather_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_inside_panel_0
                                        objectName: "mesh_Body_mat_inside_panel_0"
                                        source: "mesh_Body_mat_inside_panel_0_mesh.mesh"
                                        materials: [
                                            mat_inside_panel_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_salon_0
                                        objectName: "mesh_Body_mat_salon_0"
                                        source: "mesh_Body_mat_salon_0_mesh.mesh"
                                        materials: [
                                            mat_salon_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_rubber_0
                                        objectName: "mesh_Body_mat_rubber_0"
                                        source: "mesh_Body_mat_rubber_0_mesh.mesh"
                                        materials: [
                                            mat_rubber_material
                                        ]
                                    }
                                }
                            }
                            Node {
                                id: steeringWheel
                                objectName: "SteeringWheel"
                                x: -0.3586313724517822
                                y: 0.48624327778816223
                                z: 0.6277444362640381
                                rotation: Qt.quaternion(0.982149, -0.188104, 5.23072e-06, -2.73112e-05)
                                Node {
                                    id: mesh_SteeringWheel
                                    objectName: "mesh_SteeringWheel"
                                    x: 5.551115123125783e-17
                                    y: -5.551115123125783e-17
                                    z: -1.7881393432617188e-07
                                    rotation: Qt.quaternion(1, 1.49012e-08, -5.69907e-13, 4.13465e-12)
                                    Node {
                                        id: object_149
                                        objectName: "Object_149"
                                        x: 0.35867348313331604
                                        y: -0.658704400062561
                                        z: -0.38540735840797424
                                        rotation: Qt.quaternion(0.982149, 0.188104, -5.20843e-06, 2.73069e-05)
                                        Model {
                                            id: mesh_SteeringWheel_mat_inside_gauges_0
                                            objectName: "mesh_SteeringWheel_mat_inside_gauges_0"
                                            source: "mesh_SteeringWheel_mat_inside_gauges_0_mesh.mesh"
                                            materials: [
                                                mat_inside_gauges_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_SteeringWheel_mat_leather_0
                                            objectName: "mesh_SteeringWheel_mat_leather_0"
                                            source: "mesh_SteeringWheel_mat_leather_0_mesh.mesh"
                                            materials: [
                                                mat_leather_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_SteeringWheel_mat_inside_panel_0
                                            objectName: "mesh_SteeringWheel_mat_inside_panel_0"
                                            source: "mesh_SteeringWheel_mat_inside_panel_0_mesh.mesh"
                                            materials: [
                                                mat_inside_panel_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_SteeringWheel_mat_salon_0
                                            objectName: "mesh_SteeringWheel_mat_salon_0"
                                            source: "mesh_SteeringWheel_mat_salon_0_mesh.mesh"
                                            materials: [
                                                mat_salon_material
                                            ]
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Node {
        id: __materialLibrary__

        Texture {
            id: _20_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData90
            }
            objectName: "_20_texture"
        }

        Texture {
            id: _26_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData108
            }
            objectName: "_26_texture"
        }

        Texture {
            id: _3_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData41
            }
            objectName: "_3_texture"
        }

        Texture {
            id: _4_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData43
            }
            objectName: "_4_texture"
        }

        Texture {
            id: _22_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData97
            }
            objectName: "_22_texture"
        }

        Texture {
            id: _5_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData48
            }
            objectName: "_5_texture"
        }

        Texture {
            id: _6_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData50
            }
            objectName: "_6_texture"
        }

        Texture {
            id: _21_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData95
            }
            objectName: "_21_texture"
        }

        Texture {
            id: _23_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData99
            }
            objectName: "_23_texture"
        }

        Texture {
            id: _7_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData55
            }
            objectName: "_7_texture"
        }

        Texture {
            id: _8_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData57
            }
            objectName: "_8_texture"
        }

        Texture {
            id: _9_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData59
            }
            objectName: "_9_texture"
        }

        Texture {
            id: _27_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData113
            }
            objectName: "_27_texture"
        }

        Texture {
            id: _10_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData64
            }
            objectName: "_10_texture"
        }

        Texture {
            id: _12_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData68
            }
            objectName: "_12_texture"
        }

        Texture {
            id: _24_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData104
            }
            objectName: "_24_texture"
        }

        Texture {
            id: _13_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData73
            }
            objectName: "_13_texture"
        }

        Texture {
            id: _14_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData75
            }
            objectName: "_14_texture"
        }

        Texture {
            id: _15_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData77
            }
            objectName: "_15_texture"
        }

        Texture {
            id: _28_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData193
            }
            objectName: "_28_texture"
        }


        Texture {
            id: _17_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData81
            }
            objectName: "_17_texture"
        }

        Texture {
            id: _29_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData195
            }
            objectName: "_29_texture"
        }

        Texture {
            id: _18_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData86
            }
            objectName: "_18_texture"
        }


        Texture {
            id: _30_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData197
            }
            objectName: "_30_texture"
        }

        Texture {
            id: _16_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData
            }
            objectName: "_16_texture"
        }

        Texture {
            id: _11_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData66
            }
            objectName: "_11_texture"
        }

        Texture {
            id: _40_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData281
            }
            objectName: "_40_texture"
        }

        Texture {
            id: _31_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData202
            }
            objectName: "_31_texture"
        }

        Texture {
            id: _41_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData283
            }
            objectName: "_41_texture"
        }

        Texture {
            id: _19_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData88
            }
            objectName: "_19_texture"
        }

        Texture {
            id: _42_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData285
            }
            objectName: "_42_texture"
        }

        Texture {
            id: _32_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData204
            }
            objectName: "_32_texture"
        }

        Texture {
            id: _43_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData302
            }
            objectName: "_43_texture"
        }

        Texture {
            id: _44_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData304
            }
            objectName: "_44_texture"
        }

        Texture {
            id: _33_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData206
            }
            objectName: "_33_texture"
        }

        Texture {
            id: _45_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData306
            }
            objectName: "_45_texture"
        }

        Texture {
            id: _25_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData106
            }
            objectName: "_25_texture"
        }

        PrincipledMaterial {
            id: mat_lights_glass_material
            objectName: "mat_lights_glass"
            baseColorMap: _28_texture
            metalnessMap: _29_texture
            roughnessMap: _29_texture
            metalness: 1
            roughness: 1
            normalMap: _30_texture
            occlusionMap: _29_texture
            alphaMode: PrincipledMaterial.Blend
            clearcoatAmount: 1
            clearcoatRoughnessAmount: 0.11723333597183228
        }

        PrincipledMaterial {
            id: mat_rubber_material
            objectName: "mat_rubber"
            baseColorMap: _3_texture
            roughness: 0.3117988705635071
            normalMap: _4_texture
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: mat_lights_glass2_material
            objectName: "mat_lights_glass2"
            baseColorMap: _31_texture
            metalnessMap: _32_texture
            roughnessMap: _32_texture
            metalness: 1
            roughness: 0.48812389373779297
            normalMap: _33_texture
            occlusionMap: _32_texture
            alphaMode: PrincipledMaterial.Blend
            clearcoatAmount: 0.40300148725509644
        }

        PrincipledMaterial {
            id: mat_glass_material
            objectName: "mat_glass"
            baseColorMap: _5_texture
            metalnessMap: _6_texture
            roughnessMap: _6_texture
            metalness: 1
            occlusionMap: _6_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            clearcoatAmount: 1
            clearcoatRoughnessAmount: 0.03999999910593033
        }

        PrincipledMaterial {
            id: mat_chrome_material
            objectName: "mat_chrome"
            baseColorMap: _10_texture
            metalnessMap: _11_texture
            roughnessMap: _11_texture
            metalness: 1
            roughness: 0.038191087543964386
            normalMap: _12_texture
            occlusionMap: _11_texture
            alphaMode: PrincipledMaterial.Opaque
            clearcoatAmount: 1
            clearcoatRoughnessAmount: 0.03999999910593033
        }


        PrincipledMaterial {
            id: mat_lights_material
            objectName: "mat_lights"
            baseColorMap: _13_texture
            metalnessMap: _14_texture
            roughnessMap: _14_texture
            metalness: 1
            roughness: 1
            normalMap: _15_texture
            occlusionMap: _14_texture
            emissiveMap: _16_texture
            emissiveFactor.x: 0.5
            emissiveFactor.y: 0.5
            emissiveFactor.z: 0.5
            alphaMode: PrincipledMaterial.Blend
            transmissionFactor: 1
            transmissionMap: _17_texture
        }

        PrincipledMaterial {
            id: mat_plastic_material
            objectName: "mat_plastic"
            baseColorMap: _7_texture
            metalnessMap: _8_texture
            roughnessMap: _8_texture
            metalness: 1
            roughness: 1
            normalMap: _9_texture
            occlusionMap: _8_texture
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: mat_leather_material
            objectName: "mat_leather"
            baseColorMap: _18_texture
            metalnessMap: _19_texture
            roughnessMap: _19_texture
            metalness: 1
            roughness: 1
            normalMap: _20_texture
            occlusionMap: _19_texture
            alphaMode: PrincipledMaterial.Opaque
        }


        PrincipledMaterial {
            id: mat_inside_panel_material
            objectName: "mat_inside_panel"
            baseColorMap: _21_texture
            metalnessMap: _22_texture
            roughnessMap: _22_texture
            metalness: 1
            roughness: 1
            normalMap: _23_texture
            occlusionMap: _22_texture
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: mat_salon_material
            objectName: "mat_salon"
            baseColorMap: _24_texture
            metalnessMap: _25_texture
            roughnessMap: _25_texture
            metalness: 1
            roughness: 1
            normalMap: _26_texture
            occlusionMap: _25_texture
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: mat_body_material
            objectName: "mat_body"
            baseColor: "#ff646464"
            metalnessMap: _27_texture
            roughnessMap: _27_texture
            metalness: 1
            roughness: 0.5306851267814636
            occlusionMap: _27_texture
            alphaMode: PrincipledMaterial.Opaque
            clearcoatAmount: 1
            clearcoatRoughnessAmount: 0.02603074163198471
        }

        PrincipledMaterial {
            id: mat_bottom_material
            objectName: "mat_bottom"
            baseColorMap: _40_texture
            metalnessMap: _41_texture
            roughnessMap: _41_texture
            metalness: 1
            roughness: 1
            normalMap: _42_texture
            occlusionMap: _41_texture
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: mat_inside_gauges_material
            objectName: "mat_inside_gauges"
            baseColorMap: _43_texture
            metalnessMap: _44_texture
            roughnessMap: _44_texture
            metalness: 1
            roughness: 1
            normalMap: _45_texture
            occlusionMap: _44_texture
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: material_22_material
            objectName: "Material_22"
            baseColor: "#ff060606"
            metalness: 0.6262876391410828
            roughness: 0.5017127394676208
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
        }
    }

    PerspectiveCamera {
        id: perspectiveCamera1
        y: 250//160
        eulerRotation.z: 0
        eulerRotation.y: 0
        eulerRotation.x: -90
        z: -20//5
    }

    SpotLight {
        id: spotlight
        x: -0
        y: 594.629
        innerConeAngle: 51
        coneAngle: 83
        eulerRotation.z: 180
        eulerRotation.y: -180
        eulerRotation.x: -90
        brightness: 30
        z: -0.0002
    }

    // Sport_car_seat {
    //     id: sport_car_seat
    //     x: -14
    //     y: -34.622
    //     eulerRotation.z: 0
    //     eulerRotation.y: 180
    //     eulerRotation.x: -0.00004
    //     scale.z: 0.40
    //     scale.y: 0.40
    //     scale.x: 0.40
    //     z: 16.9
    // }

    // Sport_car_seat {
    //     id: sport_car_seat1
    //     x: 9.426
    //     y: -34.622
    //      z: 16.9
    //     scale.z: 0.4
    //     scale.y: 0.4
    //     scale.x: 0.4
    //     eulerRotation.z: 0
    //     eulerRotation.y: 180
    //     eulerRotation.x: -0.00004
    // }

    // Sport_car_seat {
    //     id: sport_car_seat2
    //     x: -12.77
    //     y: -32.252
    //     z: 45.17472
    //     scale.z: 0.38
    //     scale.y: 0.38
    //     scale.x: 0.38
    //     eulerRotation.z: 0
    //     eulerRotation.y: 180
    //     eulerRotation.x: -0.00004
    // }

    // Sport_car_seat {
    //     id: sport_car_seat3
    //     x: 9.16
    //     y: -32.252
    //     z: 45.17472
    //     scale.z: 0.38
    //     scale.y: 0.38
    //     scale.x: 0.38
    //     eulerRotation.z: 0
    //     eulerRotation.y: 180
    //     eulerRotation.x: 0
    // }

}

/*##^##
Designer {
    D{i:0;cameraSpeed3d:4;cameraSpeed3dMultiplier:1}
}
##^##*/
