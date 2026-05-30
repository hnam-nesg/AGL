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
%%{init: {
  "flowchart": {
    "htmlLabels": true,
    "nodeSpacing": 80,
    "rankSpacing": 90,
    "curve": "basis"
  },
  "themeVariables": {
    "fontSize": "22px",
    "fontFamily": "Arial"
  }
}}%%

flowchart LR
    AGL["<b style='font-size:30px'>AGL</b><br/><span style='font-size:20px'>Automotive Grade Linux</span>"]

    AGL --> AI["<b style='font-size:26px'>recipes-ai</b><br/><span style='font-size:18px'>AI / Voice / NLP Layer</span>"]
    AGL --> APP["<b style='font-size:26px'>recipes-app</b><br/><span style='font-size:18px'>IVI Application Layer</span>"]
    AGL --> RPI["<b style='font-size:26px'>Raspberry Pi 5</b><br/><span style='font-size:18px'>Main IVI Target</span>"]

    AI --> LLAMA["llama.cpp<br/>LLM Runtime"]
    AI --> PIPER["Piper<br/>Text-to-Speech"]
    AI --> PHOBERT["phoBERT<br/>Vietnamese NLP"]
    AI --> ZIP["Zipformer-hynt<br/>(sherpa-onnx)"]
    AI --> WAKE["Openwakeword<br/>Wake Word Detection"]
    AI --> VIENEU["VieNeu<br/>Vietnamese AI / TTS"]

    APP --> STEERING["Steering"]
    APP --> LIGHT["Light"]
    APP --> HVAC["HVAC"]
    APP --> MEDIA["Media"]
    APP --> SETTINGS["Settings"]
    APP --> PHONE["Phone"]
    APP --> NAV["Navigation"]
    APP --> DRIVEMODE["Drivemode"]
    APP --> PLYMOUTH["Plymouth<br/>Boot Splash"]

    RPI --> MAIN["<b>Main IVI System</b>"]

    classDef root fill:#111827,color:#ffffff,stroke:#000000,stroke-width:3px,font-size:24px;
    classDef group fill:#2563eb,color:#ffffff,stroke:#1e40af,stroke-width:3px,font-size:22px;
    classDef item fill:#e0f2fe,color:#0f172a,stroke:#0284c7,stroke-width:2px,font-size:20px;
    classDef target fill:#16a34a,color:#ffffff,stroke:#166534,stroke-width:3px,font-size:22px;

    class AGL root;
    class AI,APP group;
    class LLAMA,PIPER,PHOBERT,ZIP,WAKE,VIENEU,STEERING,LIGHT,HVAC,MEDIA,SETTINGS,PHONE,NAV,DRIVEMODE,PLYMOUTH item;
    class RPI,MAIN target;
```
