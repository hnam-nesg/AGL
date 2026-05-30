# Automotive Grade Linux OS (create Cluster)

**AGL repo source code**
```

$ mkdir AGL_Demo
$ cd AGL_Demo
$ repo init -u https://github.com/automotive-grade-linux/AGL-repo.git -b trout -m trout_20.0.0.xml
$ repo sync -j$(nproc)

```

**Build for Raspberrypi5**

```
$ source meta-agl/scripts/aglsetup.sh -f -m raspberrypi5 agl-demo agl-devel
$ cd conf
$ nano local.conf
```
After opening the local.conf file, insert the following content into the file. Then run the bitbake command to build the image
```
$ MACHINE="raspberrypi5"
$ bitbake agl-cluster-demo-qt
```

Check whether the file "agl-ivi-demo-qt-raspberrypi5.rootfs.wic.xz" has been successfully generated. Then you can use "balenaEtcher" on Windows or the "bmap tool" to write the image to an SD card or SSD
If using the bmap tool, execute the following command. (X) is the flash device name obtained using the lsblk command
```
$ sudo apt update
$ sudo apt install bmap-tools
$ lsblk
$ sudo umount /dev/sdX*
$ sudo bmaptool copy agl-cluster-demo-qt-raspberrypi5.rootfs.wic.xz /dev/sdX
```
```mermaid
flowchart TD
    AGL["AGL<br/>Automotive Grade Linux"]

    AGL --> APP["recipes-app"]
    AGL --> RPI["Raspberry Pi 5<br/>(Main)"]

    APP --> DASH["cluster-dashboard<br/>Digital Cluster UI"]
    APP --> RECEIVER["cluster-receiver<br/>CAN / Serial Receiver"]
    APP --> PLYMOUTH["plymouth<br/>Boot Splash"]

    RPI --> MEGA["Arduino Mega<br/>(Node)"]
    RPI --> UNO["Arduino Uno<br/>(Node)"]

    MEGA --> CAN1["MCP2515<br/>CAN Module"]
    UNO --> CAN2["MCP2515<br/>CAN Module"]

    CAN1 --> CANBUS["CAN Bus"]
    CAN2 --> CANBUS

    CANBUS --> RECEIVER

    classDef root fill:#1f2937,color:#ffffff,stroke:#111827,stroke-width:2px;
    classDef group fill:#2563eb,color:#ffffff,stroke:#1e40af,stroke-width:2px;
    classDef app fill:#e0f2fe,color:#0f172a,stroke:#0284c7,stroke-width:1px;
    classDef target fill:#16a34a,color:#ffffff,stroke:#166534,stroke-width:2px;
    classDef node fill:#fef3c7,color:#0f172a,stroke:#d97706,stroke-width:1px;
    classDef can fill:#fee2e2,color:#0f172a,stroke:#dc2626,stroke-width:1px;

    class AGL root;
    class APP group;
    class DASH,RECEIVER,PLYMOUTH app;
    class RPI target;
    class MEGA,UNO node;
    class CAN1,CAN2,CANBUS can;
```
