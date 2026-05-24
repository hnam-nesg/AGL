import QtQuick
import QtQuick3D

import QtQuick.Timeline

Node {
    id: node

    property alias rootNode: rootNode
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
    property url textureData218: "image://shared/textureData218"
    property url textureData204: "image://shared/textureData204"
    property url textureData220: "image://shared/textureData220"
    property url textureData55: "image://shared/textureData55"
    property url textureData222: "image://shared/textureData222"
    property url textureData57: "image://shared/textureData57"
    property url textureData227: "image://shared/textureData227"
    property url textureData59: "image://shared/textureData59"
    property url textureData99: "image://shared/textureData99"
    property url textureData229: "image://shared/textureData229"
    property url textureData231: "image://shared/textureData231"
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
    property url textureData403: "image://shared/textureData403"
    property url textureData405: "image://shared/textureData405"
    property url textureData407: "image://shared/textureData407"
    property url textureData413: "image://shared/textureData413"
    property url textureData415: "image://shared/textureData415"

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
                                id: ground_001
                                x: 0.033
                                objectName: "ground.001"
                                y: -0.01
                                scale.z: 4.5
                                scale.y: 4.5
                                scale.x: 4.5
                                z: -3.67321
                                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                                Model {
                                    id: ground_001_SOL__shadow_0
                                    x: -0
                                    y: -0.763
                                    objectName: "ground.001_SOL._shadow_0"
                                    source: "ground_001_SOL__shadow_0_mesh.mesh"
                                    eulerRotation.z: 0
                                    eulerRotation.y: -90
                                    eulerRotation.x: 90
                                    z: 0.00374
                                    materials: [
                                        sol__shadow_material
                                    ]
                                }
                            }
                            Node {
                                id: ground
                                x: 0.069
                                objectName: "ground"
                                y: 0.012
                                scale.z: 5.26556
                                scale.y: 5.26556
                                scale.x: 5.26556
                                z: -0.18977
                                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                                Model {
                                    id: ground_SOL_0
                                    x: 0.001
                                    y: 0.001
                                    objectName: "ground_SOL_0"
                                    source: "ground_SOL_0_mesh.mesh"
                                    eulerRotation.z: 90
                                    eulerRotation.x: 90
                                    eulerRotation.y: -180
                                    z: -0.00007
                                    materials: [
                                        material_material
                                    ]
                                }
                            }
                            Node {
                                id: doorFL
                                objectName: "DoorFL"
                                x: -0.9097139835357666
                                y: 0.8886669874191284
                                z: 0.5428969860076904
                                Node {
                                    id: mesh_DoorFL
                                    eulerRotation.y: -0.00001
                                    eulerRotation.x: -0
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
                                    eulerRotation.z: -0
                                    eulerRotation.y: -0
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
                                id: wheelFL
                                objectName: "WheelFL"
                                x: -0.7670765519142151
                                y: 1.4805703163146973
                                z: 0.132601797580719
                                Node {
                                    id: mesh_WheelFL
                                    objectName: "mesh_WheelFL"
                                    Node {
                                        id: object_85
                                        objectName: "Object_85"
                                        x: 0.7670765519142151
                                        y: -1.7488422393798828
                                        z: 0.3803955316543579
                                        Model {
                                            id: mesh_WheelFL_mat_wheels_0
                                            objectName: "mesh_WheelFL_mat_wheels_0"
                                            source: "mesh_WheelFL_mat_wheels_0_mesh.mesh"
                                            materials: [
                                                mat_wheels_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_WheelFL_mat_tires_0
                                            objectName: "mesh_WheelFL_mat_tires_0"
                                            source: "mesh_WheelFL_mat_tires_0_mesh.mesh"
                                            materials: [
                                                mat_tires_material
                                            ]
                                        }
                                    }
                                }
                            }
                            Node {
                                id: wheelRL
                                objectName: "WheelRL"
                                x: -0.7670765519142151
                                y: -1.384366512298584
                                z: 0.132601797580719
                                Node {
                                    id: mesh_WheelRR
                                    objectName: "mesh_WheelRR"
                                    x: -0.03472226858139038
                                    Node {
                                        id: object_95
                                        objectName: "Object_95"
                                        x: 0.8017988204956055
                                        y: 1.1160945892333984
                                        z: 0.39532244205474854
                                        Model {
                                            id: mesh_WheelRR_mat_wheels_0
                                            objectName: "mesh_WheelRR_mat_wheels_0"
                                            source: "mesh_WheelRR_mat_wheels_0_mesh.mesh"
                                            materials: [
                                                mat_wheels_material
                                            ]
                                        }
                                        Model {
                                            id: mesh_WheelRR_mat_tires_0
                                            objectName: "mesh_WheelRR_mat_tires_0"
                                            source: "mesh_WheelRR_mat_tires_0_mesh.mesh"
                                            materials: [
                                                mat_tires_material
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
                                    Model {
                                        id: mesh_Body_mat_body_0
                                        objectName: "mesh_Body_mat_body_0"
                                        source: "mesh_Body_mat_body_0_mesh.mesh"
                                        materials: mat_body_material
                                    }
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
                                            material_8_material
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
                                        materials: material_22_material
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
            id: _34_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData218
            }
            objectName: "_34_texture"
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
            id: _35_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData220
            }
            objectName: "_35_texture"
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
            id: _36_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData222
            }
            objectName: "_36_texture"
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
            id: _37_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData227
            }
            objectName: "_37_texture"
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
            id: _38_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData229
            }
            objectName: "_38_texture"
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
            id: _39_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData231
            }
            objectName: "_39_texture"
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

        Texture {
            id: _52_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData403
            }
            objectName: "_22_texture"
        }

        Texture {
            id: _53_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData405
            }
            objectName: "_23_texture"
        }

        Texture {
            id: _54_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData407
            }
            objectName: "_24_texture"
        }

        Texture {
            id: _55_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData413
            }
            objectName: "_25_texture"
        }

        Texture {
            id: _56_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData415
            }
            objectName: "_26_texture"
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
            id: mat_wheels_material
            objectName: "mat_wheels"
            baseColorMap: _34_texture
            metalnessMap: _35_texture
            roughnessMap: _35_texture
            metalness: 1
            roughness: 1
            normalMap: _36_texture
            occlusionMap: _35_texture
            alphaMode: PrincipledMaterial.Opaque
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
            id: mat_tires_material
            baseColorMap: _37_texture
            objectName: "mat_tires"
            metalnessMap: _38_texture
            roughnessMap: _38_texture
            metalness: 0.2327566295862198
            roughness: 1
            normalMap: _39_texture
            occlusionMap: _38_texture
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

        PrincipledMaterial {
            id: material_material
            objectName: "material"
            baseColor: "#54ffffff"
            baseColorMap: _52_texture
            metalnessMap: _53_texture
            roughnessMap: _53_texture
            metalness: 1
            roughness: 0.8040408492088318
            normalMap: _54_texture
            occlusionMap: _53_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
        }

        PrincipledMaterial {
            id: sol__shadow_material
            objectName: "SOL._shadow"
            baseColor: "#ff000000"
            baseColorMap: _55_texture
            roughness: 0.8040408492088318
            normalMap: _56_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
        }

        PrincipledMaterial {
            id: material_8
            objectName: "Material_8"
            baseColor: "transparent"
            metalness: 1
            roughness: 0.25180724263191223
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
        }

        PrincipledMaterial {
            id: material_8_material
            objectName: "Material_8"
            baseColor: "#4f333333"
            metalness: 1
            roughness: 0.25180724263191223
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
        }

    }

    PerspectiveCamera {
        id: perspectiveCamera
        x: -45
        y: 40
        eulerRotation.z: -1.0184
        eulerRotation.y: -140
        eulerRotation.x: -7
        z: -110
    }

    Model {
        id: sphere
        x: -23
        y: 20
        source: "#Sphere"
        scale.z: 0.02313
        scale.y: 0.02313
        scale.x: 0.02313
        z: -60
        materials: material_8
    }

    Model {
        id: cube
        x: -17.49
        y: 19.153
        source: "#Sphere"
        z: -62.23964
        scale.z: 0.00945
        scale.y: 0.00945
        scale.x: 0.00945
        eulerRotation.x: -90
        materials: material_8
    }
}

/*##^##
Designer {
    D{i:0;cameraSpeed3d:4;cameraSpeed3dMultiplier:1}
}
##^##*/
