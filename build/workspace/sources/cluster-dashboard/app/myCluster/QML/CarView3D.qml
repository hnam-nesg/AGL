import QtQuick
import QtQuick3D

Item {
    id: control

    property string gearMode: "P"
    property bool enable3DAnimation: true
    property bool highQuality: true
    property bool interactive: true
    property int cameraTransitionDuration: 0
    property int readyWarmupFrames: 12
    property string assetRootUrl: typeof externalAssetRootUrl === "undefined" ? "qrc:/asset/" : externalAssetRootUrl
    property var activeCamera: gearMode === "P" || gearMode === "N" ? carModel.activeCamera1 : carModel.activeCamera2
    property bool modelReady: false
    property int warmupFrameCount: 0
    readonly property bool hasFpsLogger: typeof fpsLogger !== "undefined" && fpsLogger !== null
    readonly property string fpsLabel: "CarView3D gear=" + gearMode
                                       + " animation=" + enable3DAnimation
                                       + " highQuality=" + highQuality
                                       + " assetRoot=" + assetRootUrl

    function assetUrl(path) {
        return assetRootUrl + path
    }

    function logFpsMark(message) {
        if (hasFpsLogger)
            fpsLogger.mark(message)
    }

    function resetModelReadiness(reason) {
        modelReady = false
        warmupFrameCount = 0
        logFpsMark("modelReady=false reason=" + reason)
    }

    Component.onCompleted: {
        if (hasFpsLogger)
            fpsLogger.start(fpsLabel)
    }

    Component.onDestruction: {
        if (hasFpsLogger)
            fpsLogger.stop()
    }

    onGearModeChanged: logFpsMark("gearMode=" + gearMode)
    onEnable3DAnimationChanged: logFpsMark("enable3DAnimation=" + enable3DAnimation)
    onHighQualityChanged: {
        logFpsMark("highQuality=" + highQuality)
        resetModelReadiness("qualityChanged")
    }
    onAssetRootUrlChanged: {
        logFpsMark("assetRoot=" + assetRootUrl)
        resetModelReadiness("assetRootChanged")
    }

    function showFrontView() {
        carModel.keyFrame.value = 360
        view3d.state = "frontView"
    }

    function showRearView() {
        carModel.keyFrame.value = 0
        resetRotation()
        view3d.state = "rearView"
    }

    function setKeyFrame(value) {
        carModel.keyFrame.value = value
    }

    function resetRotation() {
        carModel.rootNode.eulerRotation = Qt.vector3d(0, 0, 0)
    }

    View3D {
        id: view3d
        anchors.fill: parent

        Texture {
            id: environmentProbe
            source: control.highQuality ? control.assetUrl("qwantani_mid_morning_puresky_1k.hdr") : ""
        }

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            clearColor: "transparent"
            lightProbe: control.highQuality ? environmentProbe : null
            antialiasingMode: control.highQuality ? SceneEnvironment.MSAA : SceneEnvironment.NoAA
        }

        Scene {
            id: carModel
            scale: Qt.vector3d(10, 10, 10)
            assetRootUrl: control.assetRootUrl
            highQuality: control.highQuality
            autoRotate: control.enable3DAnimation
                        && (control.gearMode === "P" || control.gearMode === "N")
                        && !view3dmouse.dragActive
        }

        Component.onCompleted: {
            console.log("assetRootUrl =", control.assetRootUrl)
            console.log("HDR source =", environmentProbe.source)
        }

        MouseArea {
            id: view3dmouse
            anchors.fill: parent
            enabled: control.interactive

            property real lastX: 0
            property bool dragActive: false

            onPressed: {
                lastX = mouse.x
                dragActive = true
            }

            onReleased: {
                dragActive = false
            }

            onCanceled: {
                dragActive = false
            }

            onPositionChanged: {
                let dx = mouse.x - lastX
                lastX = mouse.x
                carModel.rootNode.eulerRotation = Qt.vector3d(
                    carModel.rootNode.eulerRotation.x,
                    carModel.rootNode.eulerRotation.y + dx * 0.5,
                    carModel.rootNode.eulerRotation.z
                )
            }
        }

        camera: control.activeCamera
        states: [
            State {
                name: "frontView"
                PropertyChanges {
                    target: control.activeCamera
                    position.x: carModel.activeCamera1.position.x
                    position.y: carModel.activeCamera1.position.y
                    position.z: carModel.activeCamera1.position.z
                    eulerRotation.x: carModel.activeCamera1.eulerRotation.x
                    eulerRotation.y: carModel.activeCamera1.eulerRotation.y
                    eulerRotation.z: carModel.activeCamera1.eulerRotation.z
                }
            },
            State {
                name: "rearView"
                PropertyChanges {
                    target: control.activeCamera
                    position.x: carModel.activeCamera2.position.x
                    position.y: carModel.activeCamera2.position.y
                    position.z: carModel.activeCamera2.position.z
                    eulerRotation.x: carModel.activeCamera2.eulerRotation.x
                    eulerRotation.y: carModel.activeCamera2.eulerRotation.y
                    eulerRotation.z: carModel.activeCamera2.eulerRotation.z
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "*"
                ParallelAnimation {
                    NumberAnimation {
                        properties: "position.x, position.y, position.z"
                        duration: control.cameraTransitionDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        properties: "eulerRotation.x, eulerRotation.y, eulerRotation.z"
                        duration: control.cameraTransitionDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        ]
    }

    Connections {
        target: carModel

        function onAutoRotateChanged() {
            control.logFpsMark("autoRotate=" + carModel.autoRotate)
        }

    }

    FrameAnimation {
        id: fpsFrameCounter
        running: control.hasFpsLogger && control.visible && view3d.visible && carModel.autoRotate

        onRunningChanged: control.logFpsMark("fpsCounterRunning=" + running)
        onTriggered: fpsLogger.frame()
    }

    FrameAnimation {
        id: modelReadyFrameCounter
        running: control.visible && view3d.visible && !control.modelReady

        onTriggered: {
            control.warmupFrameCount += 1
            if (control.warmupFrameCount >= control.readyWarmupFrames) {
                control.modelReady = true
                control.logFpsMark("modelReady=true warmupFrames=" + control.warmupFrameCount)
            }
        }
    }
}
