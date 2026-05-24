import QtQuick
import QtQuick3D

Node {
    id: node

    Node {
        id: rootNode
        objectName: "RootNode"
        Model {
            id: seay_backrest
            objectName: "seay backrest"
            x: -5.847
            y: 158.429
            z: 17.81886
            scale.x: 100
            scale.y: 100
            scale.z: 100
            source: "seay_backrest_mesh.mesh"
            eulerRotation.z: 0
            eulerRotation.y: -0
            eulerRotation.x: -15.06454
            materials: [
                main_lether__002_material,
            ]
        }
        Model {
            id: seat_pocket
            objectName: "Seat pocket"
            scale.x: 100
            scale.y: 100
            scale.z: 100
            source: "plane_008_mesh.mesh"
            materials: [
                main_lether__002_material
            ]
        }
        Model {
            id: seat_base
            objectName: "SEat base"
            x: -5.846825122833252
            y: 127.96637725830078
            z: 44.48672103881836
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale.x: 100
            scale.y: 100
            scale.z: 100
            source: "seat_base_mesh.mesh"
            materials: [
                main_lether__002_material
            ]
        }
    }

    Node {
        id: __materialLibrary__

        PrincipledMaterial {
            id: main_lether__002_material
            objectName: "main lether..002"
            baseColor: "#FF3C00"
            roughness: 0.67090904712677
        }
    }

    // Animations:
}

/*##^##
Designer {
    D{i:0;cameraSpeed3d:7;cameraSpeed3dMultiplier:1}
}
##^##*/
