# Automotive Grade Linux OS (create IVI)

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
$ bitbake agl-ivi-demo-qt
```

Check whether the file "agl-ivi-demo-qt-raspberrypi5.rootfs.wic.xz" has been successfully generated. Then you can use "balenaEtcher" on Windows or the "bmap tool" to write the image to an SD card or SSD
If using the bmap tool, execute the following command. (X) is the flash device name obtained using the lsblk command
```
$ sudo apt update
$ sudo apt install bmap-tools
$ lsblk
$ sudo umount /dev/sdX*
$ sudo bmaptool copy agl-ivi-demo-qt-raspberrypi5.rootfs.wic.xz /dev/sdX
```
```mermaid
flowchart TD
    AGL["AGL<br/>Automotive Grade Linux"]

    AGL --> AI["recipes-ai"]
    AGL --> APP["recipes-app"]
    AGL --> RPI["Raspberry Pi 5<br/>(Main Target)"]

    AI --> LLAMA["llama.cpp<br/>LLM Runtime"]
    AI --> PIPER["piper<br/>Text-to-Speech"]
    AI --> PHOBERT["phoBERT<br/>Vietnamese NLP"]
    AI --> ZIP["zipformer-hynt<br/>(sherpa-onnx)"]
    AI --> WAKE["openwakeword<br/>Wake Word Detection"]
    AI --> VIENEU["VieNeu<br/>Vietnamese AI/TTS"]

    APP --> STEERING["steering"]
    APP --> LIGHT["light"]
    APP --> HVAC["hvac"]
    APP --> MEDIA["media"]
    APP --> SETTINGS["settings"]
    APP --> PHONE["phone"]
    APP --> NAV["navigation"]
    APP --> DRIVEMODE["drivemode"]
    APP --> PLYMOUTH["plymouth<br/>Boot Splash"]

    RPI --> MAIN["Main IVI System"]

    classDef root fill:#1f2937,color:#ffffff,stroke:#111827,stroke-width:2px;
    classDef group fill:#2563eb,color:#ffffff,stroke:#1e40af,stroke-width:2px;
    classDef item fill:#e0f2fe,color:#0f172a,stroke:#0284c7,stroke-width:1px;
    classDef target fill:#16a34a,color:#ffffff,stroke:#166534,stroke-width:2px;

    class AGL root;
    class AI,APP group;
    class LLAMA,PIPER,PHOBERT,ZIP,WAKE,VIENEU,STEERING,LIGHT,HVAC,MEDIA,SETTINGS,PHONE,NAV,DRIVEMODE,PLYMOUTH item;
    class RPI,MAIN target;
```
