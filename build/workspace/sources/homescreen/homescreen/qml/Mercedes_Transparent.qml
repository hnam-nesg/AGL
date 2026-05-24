import QtQuick
import QtQuick3D

import QtQuick.Timeline

Node {
    id: node

    property alias rootNode: rootNode

    // Resources
    property url textureData227: "image://shared/textureData227"
    property url textureData229: "image://shared/textureData229"
    property url textureData231: "image://shared/textureData231"

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
                                id: wheelFR
                                objectName: "WheelFR"
                                x: 0.7670770287513733
                                y: 1.4805703163146973
                                z: 0.132601797580719
                                Node {
                                    id: mesh_WheelFR
                                    objectName: "mesh_WheelFR"
                                    Node {
                                        id: object_90
                                        objectName: "Object_90"
                                        x: -0.7670770287513733
                                        y: -1.7488422393798828
                                        z: 0.3803955316543579
                                        Model {
                                            id: mesh_WheelFR_mat_tires_0
                                            objectName: "mesh_WheelFR_mat_tires_0"
                                            source: "mesh_WheelFR_mat_tires_0_mesh.mesh"
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
                                id: wheelRR
                                objectName: "WheelRR"
                                x: 0.7670770287513733
                                y: -1.384366512298584
                                z: 0.132601797580719
                                Node {
                                    id: mesh_WheelRR247
                                    objectName: "mesh_WheelRR"
                                    x: 0.03472185134887695
                                    Node {
                                        id: object_100
                                        objectName: "Object_100"
                                        x: -0.8017988801002502
                                        y: 1.1160943508148193
                                        z: 0.3953222632408142
                                        Model {
                                            id: mesh_WheelRR_mat_tires_0251
                                            objectName: "mesh_WheelRR_mat_tires_0"
                                            source: "mesh_WheelRR_mat_tires_0_mesh252.mesh"
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
                                        materials: material_8_material
                                    }
                                    Model {
                                        id: mesh_Body_mat_bottom_0
                                        objectName: "mesh_Body_mat_bottom_0"
                                        source: "mesh_Body_mat_bottom_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_glass_0
                                        objectName: "mesh_Body_mat_glass_0"
                                        source: "mesh_Body_mat_glass_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_plastic_0
                                        objectName: "mesh_Body_mat_plastic_0"
                                        source: "mesh_Body_mat_plastic_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_chrome_0
                                        objectName: "mesh_Body_mat_chrome_0"
                                        source: "mesh_Body_mat_chrome_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_lights_glass_0
                                        objectName: "mesh_Body_mat_lights_glass_0"
                                        source: "mesh_Body_mat_lights_glass_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_lights_glass2_0
                                        objectName: "mesh_Body_mat_lights_glass2_0"
                                        source: "mesh_Body_mat_lights_glass2_0_mesh.mesh"
                                        materials: [
                                            material_8_material
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
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_leather_0
                                        objectName: "mesh_Body_mat_leather_0"
                                        source: "mesh_Body_mat_leather_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_inside_panel_0
                                        objectName: "mesh_Body_mat_inside_panel_0"
                                        source: "mesh_Body_mat_inside_panel_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_salon_0
                                        objectName: "mesh_Body_mat_salon_0"
                                        source: "mesh_Body_mat_salon_0_mesh.mesh"
                                        materials: [
                                            material_8_material
                                        ]
                                    }
                                    Model {
                                        id: mesh_Body_mat_rubber_0
                                        objectName: "mesh_Body_mat_rubber_0"
                                        source: "mesh_Body_mat_rubber_0_mesh.mesh"
                                        materials: [
                                            material_8_material
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
            id: _37_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData227
            objectName: "_37_texture"
        }

        Texture {
            id: _38_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData229
            objectName: "_38_texture"
        }

        Texture {
            id: _39_texture
            generateMipmaps: true
            mipFilter: Texture.Linear
            source: node.textureData231
            objectName: "_39_texture"
        }

        PrincipledMaterial {
            id: mat_tires_material
            objectName: "mat_tires"
            baseColor: "#02b600"
            //baseColorMap: _37_texture
            metalnessMap: _38_texture
            roughnessMap: _38_texture
            metalness: 0.2327566295862198
            roughness: 1
            normalMap: _39_texture
            occlusionMap: _38_texture
            alphaMode: PrincipledMaterial.Opaque
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
        id: perspectiveCamera2
        y: -230
        eulerRotation.z: 0
        eulerRotation.y: 0
        eulerRotation.x: 90
        z: 30
    }

}

/*##^##
Designer {
    D{i:0;cameraSpeed3d:4;cameraSpeed3dMultiplier:1}
}
##^##*/
