import QtQuick
import QtQuick3D

import QtQuick.Timeline

Node {
    id: node

    property alias rootNode: rootNode

    // Resources

    property alias doorFL: doorFL
    property alias doorFR: doorFR
    property alias doorRL: doorRL
    property alias doorRR: doorRR

    property url textureData: "textureData.jpg"
    property url textureData104: "textureData104.jpg"
    property url textureData106: "textureData106.png"
    property url textureData90: "textureData90.png"
    property url textureData86: "textureData86.jpg"
    property url textureData108: "textureData108.png"
    property url textureData50: "textureData50.png"
    property url textureData88: "textureData88.png"
    property url textureData41: "textureData41.jpg"
    property url textureData43: "textureData43.png"
    property url textureData48: "textureData48.png"
    property url textureData202: "textureData202.png"
    property url textureData206: "textureData206.png"
    property url textureData204: "textureData204.png"
    property url textureData55: "textureData55.jpg"
    property url textureData57: "textureData57.png"
    property url textureData59: "textureData59.png"
    property url textureData77: "textureData77.png"
    property url textureData113: "textureData113.png"
    property url textureData73: "textureData73.png"
    property url textureData75: "textureData75.png"
    property url textureData95: "textureData95.jpg"
    property url textureData97: "textureData97.png"
    property url textureData99: "textureData99.png"
    property url textureData81: "textureData81.png"
    property url textureData64: "textureData64.jpg"
    property url textureData68: "textureData68.png"
    property url textureData193: "textureData193.png"
    property url textureData195: "textureData195.png"
    property url textureData197: "textureData197.png"
    property url textureData66: "textureData66.png"

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
                                }
                            }
                            Node {
                                id: doorFL
                                objectName: "DoorFL"
                                x: -0.9097139835357666
                                y: 0.8886669874191284
                                z: 0.5428969860076904
                                // SequentialAnimation on eulerRotation.z{
                                //     running: Handler3D.door
                                //     PropertyAnimation {from: 0; to: -30; duration: 1500; easing.type: Easing.InOutQuad}
                                //     PropertyAnimation {from: -30; to: 0; duration: 1500; easing.type: Easing.InOutQuad}
                                // }
                                Behavior on eulerRotation.z{NumberAnimation{duration: 1500; easing.type: Easing.Linear}}
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
                                // SequentialAnimation on eulerRotation.z{
                                //     running: Handler3D.door
                                //     PropertyAnimation {from: 0; to: 30; duration: 1500; easing.type: Easing.InOutQuad}
                                //     PropertyAnimation {from: 30; to: 0; duration: 1500; easing.type: Easing.InOutQuad}
                                // }
                                Behavior on eulerRotation.z{NumberAnimation{duration: 1500; easing.type: Easing.Linear}}
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
                                // SequentialAnimation on eulerRotation.z{
                                //     running: Handler3D.door
                                //     PropertyAnimation {from: 0; to: -30; duration: 1500; easing.type: Easing.InOutQuad}
                                //     PropertyAnimation {from: -30; to: 0; duration: 1500; easing.type: Easing.InOutQuad}
                                // }
                                Behavior on eulerRotation.z{NumberAnimation{duration: 1500; easing.type: Easing.Linear}}
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
                                // SequentialAnimation on eulerRotation.z{
                                //     running: Handler3D.door
                                //     PropertyAnimation {from: 0; to: 30; duration: 1500; easing.type: Easing.InOutQuad}
                                //     PropertyAnimation {from: 30; to: 0; duration: 1500; easing.type: Easing.InOutQuad}
                                // }
                                Behavior on eulerRotation.z{NumberAnimation{duration: 1500; easing.type: Easing.Linear}}
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
                                    Behavior on eulerRotation.x{
                                        NumberAnimation{duration: 1000; easing.type: Easing.InOutQuad}
                                    }
                                    eulerRotation.x: Handler3D.capo ? 45 : 0
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
                                    Behavior on eulerRotation.x{
                                        NumberAnimation{duration: 2000; easing.type: Easing.Linear}
                                    }
                                    eulerRotation.x: Handler3D.trunk ? -65 : 0
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
            id: _16_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData
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
            id: _11_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData66
            }
            objectName: "_11_texture"
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
            id: _19_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData88
            }
            objectName: "_19_texture"
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
            id: _33_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            sourceItem: Image{
                source: textureData206
            }
            objectName: "_33_texture"
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
            id: _21_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData95
            objectName: "_21_texture"
        }

        Texture {
            id: _23_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData99
            objectName: "_23_texture"
        }

        Texture {
            id: _22_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData97
            objectName: "_22_texture"
        }

        Texture {
            id: _17_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData81
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
    }

    PerspectiveCamera {
        id: perspectiveCamera1
        y: 250
        eulerRotation.z: 0
        eulerRotation.y: 0
        eulerRotation.x: -90
        z: -20
    }

    // Connections{
    //     target: Handler3D
    //     function onTrunkChanged(value){
    //         mesh_Trunk.eulerRotation.x = value ? -65 : 0;
    //     }
    //     function onCapoChanged(value){
    //         mesh_Hood.eulerRotation.x = value ? 45 : 0
    //     }
    // }

}

