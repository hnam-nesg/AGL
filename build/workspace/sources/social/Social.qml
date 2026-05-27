import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.VirtualKeyboard
import QtCore
import QtWebEngine

ApplicationWindow {
    id: root
    width: 860
    height: 670
    visible: true
    color: themeSettings.shellColor
    property bool pendingAutoClickFirst: false
    property bool pendingAutoFullscreen: false
    property int autoClickAttempts: 0
    property int fullscreenAttempts: 0
    property bool autoSkipAdsEnabled: true
    property bool youtubePlaying: false

    function shouldAutoFullscreenForType(type) {
        const normalizedType = String(type).toLowerCase()
        return normalizedType === "music" ||
               normalizedType === "song" ||
               normalizedType === "artistworks" ||
               normalizedType === "artist" ||
               normalizedType === "artist_works"
    }

    function searchTextForType(type, query) {
        const normalizedType = String(type).toLowerCase()
        const trimmedQuery = String(query).trim()

        if (normalizedType === "music" || normalizedType === "song") {
            return trimmedQuery + " bài hát"
        }

        if (normalizedType === "movie" || normalizedType === "film") {
            return trimmedQuery + " phim"
        }

        if (normalizedType === "artistworks" || normalizedType === "artist" || normalizedType === "artist_works") {
            return trimmedQuery + " tuyển tập bài hát"
        }

        return trimmedQuery
    }

    function requestYoutubeSearch(type, query, autoClickFirst) {
        const searchText = searchTextForType(type, query)
        if (searchText.length === 0) {
            return
        }

        pendingAutoClickFirst = autoClickFirst
        pendingAutoFullscreen = autoClickFirst && shouldAutoFullscreenForType(type)
        autoClickAttempts = 0
        fullscreenAttempts = 0
        autoClickTimer.stop()
        autoFullscreenTimer.stop()
        web.url = "https://www.youtube.com/results?search_query=" + encodeURIComponent(searchText)
    }

    function tryClickFirstResult() {
        if (!pendingAutoClickFirst) {
            autoClickTimer.stop()
            return
        }

        autoClickAttempts += 1
        web.runJavaScript(
            "(function() {" +
            "  const selectors = [" +
            "    'ytd-video-renderer a#thumbnail'," +
            "    'ytd-video-renderer #video-title'," +
            "    'ytd-movie-renderer a'," +
            "    'ytd-rich-item-renderer a#thumbnail'," +
            "    'ytd-grid-video-renderer a#thumbnail'," +
            "    'a[href^=\"/watch\"]'" +
            "  ];" +
            "  for (const selector of selectors) {" +
            "    const nodes = Array.from(document.querySelectorAll(selector));" +
            "    const node = nodes.find(function(item) {" +
            "      const href = item.href || item.getAttribute('href') || '';" +
            "      const hidden = item.offsetParent === null && item.getClientRects().length === 0;" +
            "      return !hidden && href.indexOf('/shorts/') === -1;" +
            "    });" +
            "    if (node) {" +
            "      node.scrollIntoView({block: 'center', inline: 'center'});" +
            "      node.click();" +
            "      return true;" +
            "    }" +
            "  }" +
            "  return false;" +
            "})()",
            function(clicked) {
                if (clicked || root.autoClickAttempts >= 8) {
                    root.pendingAutoClickFirst = false
                    autoClickTimer.stop()

                    if (clicked && root.pendingAutoFullscreen) {
                        root.fullscreenAttempts = 0
                        autoFullscreenTimer.restart()
                    } else {
                        root.pendingAutoFullscreen = false
                        autoFullscreenTimer.stop()
                    }
                }
            }
        )
    }

    function tryEnterAutoFullscreen() {
        if (!pendingAutoFullscreen) {
            autoFullscreenTimer.stop()
            return
        }

        fullscreenAttempts += 1
        web.runJavaScript(
            "(function() {" +
            "  var player = document.querySelector('.html5-video-player');" +
            "  if (window.location.href.indexOf('/watch') === -1) { return JSON.stringify({state: 'waiting'}); }" +
            "  if (document.fullscreenElement || (player && player.classList.contains('ytp-fullscreen'))) {" +
            "    return JSON.stringify({state: 'entered'});" +
            "  }" +
            "  var video = document.querySelector('video');" +
            "  if (video) { video.play().catch(function() {}); }" +
            "  if (player) {" +
            "    player.classList.remove('ytp-autohide');" +
            "    player.dispatchEvent(new MouseEvent('mousemove', {bubbles: true, cancelable: true, view: window}));" +
            "  }" +
            "  var selectors = [" +
            "    '.ytp-fullscreen-button'," +
            "    'button.ytp-fullscreen-button'," +
            "    'button[aria-label*=\"Full screen\"]'," +
            "    'button[aria-label*=\"Toàn màn hình\"]'," +
            "    'button[title*=\"Full screen\"]'," +
            "    'button[title*=\"Toàn màn hình\"]'" +
            "  ];" +
            "  var button = null;" +
            "  for (var i = 0; i < selectors.length && !button; ++i) {" +
            "    button = document.querySelector(selectors[i]);" +
            "  }" +
            "  if (!button || button.disabled) { return JSON.stringify({state: 'missing-button'}); }" +
            "  var rect = button.getBoundingClientRect();" +
            "  return JSON.stringify({" +
            "    state: 'button'," +
            "    x: rect.left + rect.width / 2," +
            "    y: rect.top + rect.height / 2," +
            "    viewportWidth: window.innerWidth || document.documentElement.clientWidth || 1," +
            "    viewportHeight: window.innerHeight || document.documentElement.clientHeight || 1" +
            "  });" +
            "})()",
            function(result) {
                let payload = null
                try {
                    payload = JSON.parse(result)
                } catch (error) {
                    payload = { state: String(result) }
                }
                if (!payload || typeof payload !== "object") {
                    payload = { state: String(result) }
                }

                if (payload.state === "entered" || root.fullscreenAttempts >= 15) {
                    root.pendingAutoFullscreen = false
                    autoFullscreenTimer.stop()
                    return
                }

                if (payload.state === "button" && payload.viewportWidth > 0 && payload.viewportHeight > 0) {
                    const x = payload.x * web.width / payload.viewportWidth
                    const y = payload.y * web.height / payload.viewportHeight
                    inputInjector.clickItem(web, x, y)
                }
            }
        )
    }

    function trySkipAds() {
        if (!autoSkipAdsEnabled) {
            return
        }

        web.runJavaScript(
            "(function() {" +
            "  var clicked = false;" +
            "  var selectors = [" +
            "    '.ytp-ad-skip-button'," +
            "    '.ytp-ad-skip-button-modern'," +
            "    '.ytp-skip-ad-button'," +
            "    '.ytp-ad-overlay-close-button'," +
            "    'button[aria-label^=\"Skip\"]'," +
            "    'button[aria-label^=\"Bỏ qua\"]'," +
            "    'button[aria-label*=\"Skip ad\"]'," +
            "    'button[aria-label*=\"Bỏ qua quảng cáo\"]'" +
            "  ];" +
            "  selectors.forEach(function(selector) {" +
            "    Array.from(document.querySelectorAll(selector)).forEach(function(button) {" +
            "      if (button && !button.disabled && button.offsetParent !== null) {" +
            "        button.click();" +
            "        clicked = true;" +
            "      }" +
            "    });" +
            "  });" +
            "  Array.from(document.querySelectorAll('.ytp-ad-overlay-container, .ytp-ad-image-overlay, ytd-display-ad-renderer, ytd-promoted-sparkles-web-renderer')).forEach(function(node) {" +
            "    node.style.display = 'none';" +
            "  });" +
            "  var adShowing = document.querySelector('.ad-showing');" +
            "  var video = document.querySelector('video');" +
            "  if (adShowing && video && isFinite(video.duration) && video.duration > 0) {" +
            "    video.currentTime = Math.max(video.currentTime, video.duration - 0.2);" +
            "    clicked = true;" +
            "  }" +
            "  return clicked;" +
            "})()"
        )
    }

    function setYoutubePlaying(playing) {
        if (youtubePlaying === playing) {
            return
        }

        youtubePlaying = playing
        // qmllint disable unqualified
        socialDbus.SetYoutubePlaying(playing)
        // qmllint enable unqualified
    }

    function updateYoutubePlaybackState() {
        web.runJavaScript(
            "(function() {" +
            "  var video = document.querySelector('video');" +
            "  return !!(video && !video.paused && !video.ended && video.readyState >= 2);" +
            "})()",
            function(playing) {
                root.setYoutubePlaying(!!playing)
            }
        )
    }

    function pauseYoutubePlayback() {
        pendingAutoClickFirst = false
        pendingAutoFullscreen = false
        autoClickTimer.stop()
        autoFullscreenTimer.stop()

        web.runJavaScript(
            "(function() {" +
            "  document.documentElement.classList.remove('agl-auto-fullscreen');" +
            "  var video = document.querySelector('video');" +
            "  if (video) {" +
            "    video.pause();" +
            "    return true;" +
            "  }" +
            "  var button = document.querySelector('.ytp-play-button');" +
            "  if (button) {" +
            "    var label = (button.getAttribute('aria-label') || button.title || '').toLowerCase();" +
            "    if (label.indexOf('pause') !== -1 || label.indexOf('tạm dừng') !== -1) {" +
            "      button.click();" +
            "      return true;" +
            "    }" +
            "  }" +
            "  return false;" +
            "})()",
            function() {
                root.setYoutubePlaying(false)
            }
        )
    }

    Timer {
        id: autoClickTimer
        interval: 1000
        repeat: true
        onTriggered: root.tryClickFirstResult()
    }

    Timer {
        id: autoFullscreenTimer
        interval: 1200
        repeat: true
        onTriggered: root.tryEnterAutoFullscreen()
    }

    Timer {
        id: adSkipTimer
        interval: 800
        repeat: true
        running: root.autoSkipAdsEnabled
        onTriggered: root.trySkipAds()
    }

    Timer {
        id: youtubePlaybackTimer
        interval: 1000
        repeat: true
        running: true
        onTriggered: root.updateYoutubePlaybackState()
    }

    Connections {
        // qmllint disable unqualified
        target: socialDbus
        // qmllint enable unqualified

        function onSearchRequested(type, query, autoClickFirst) {
            root.requestYoutubeSearch(type, query, autoClickFirst)
        }

        function onPauseYoutubeRequested() {
            root.pauseYoutubePlayback()
        }
    }

    Rectangle{
        y: 5; width: 850; height: 660; color: "transparent"; radius: 10
        Background{width_:1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}
        
        BorderColor{}

        WebEngineProfile {
            id: youtubeProfile

            storageName: "social-youtube"
            persistentStoragePath: StandardPaths.writableLocation(StandardPaths.AppDataLocation) + "/youtube-profile"
            cachePath: StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/youtube-profile"

            persistentCookiesPolicy: WebEngineProfile.ForcePersistentCookies
            httpCacheType: WebEngineProfile.DiskHttpCache
        }

        Item {
            id: youtubeViewport
            anchors.fill: parent
            WebEngineView {
                id: web

                anchors.fill: parent
                profile: youtubeProfile

                backgroundColor: themeSettings.shellColor
                zoomFactor: 1.0

                settings.playbackRequiresUserGesture: false
                settings.fullScreenSupportEnabled: true
                settings.focusOnNavigationEnabled: true

                url: "https://www.youtube.com/"

                Component.onCompleted: {
                    console.log("root:", root.width, root.height)
                    console.log("youtubeViewport:", youtubeViewport.x, youtubeViewport.y, youtubeViewport.width, youtubeViewport.height)
                    console.log("web:", web.x, web.y, web.width, web.height)
                    console.log("screen DPR:", Screen.devicePixelRatio)
                }

                onLoadingChanged: function(loadRequest) {
                    console.log("YouTube load status:", loadRequest.status)

                    if (loadRequest.status === 2) {
                        if (root.pendingAutoClickFirst && String(web.url).indexOf("/results") !== -1) {
                            root.autoClickAttempts = 0
                            autoClickTimer.restart()
                        }

                        if (root.pendingAutoFullscreen && String(web.url).indexOf("/watch") !== -1) {
                            root.fullscreenAttempts = 0
                            autoFullscreenTimer.restart()
                        }

                        root.trySkipAds()

                        web.runJavaScript(
                            "JSON.stringify({" +
                            "innerWidth: window.innerWidth," +
                            "innerHeight: window.innerHeight," +
                            "devicePixelRatio: window.devicePixelRatio," +
                            "docClientWidth: document.documentElement.clientWidth," +
                            "docScrollWidth: document.documentElement.scrollWidth," +
                            "bodyClientWidth: document.body ? document.body.clientWidth : -1," +
                            "bodyScrollWidth: document.body ? document.body.scrollWidth : -1," +
                            "scrollX: window.scrollX," +
                            "visualOffsetLeft: window.visualViewport ? window.visualViewport.offsetLeft : -1," +
                            "visualWidth: window.visualViewport ? window.visualViewport.width : -1" +
                            "})",
                            function(result) {
                                console.log("WEB_METRICS:", result)
                            }
                        )
                    }
                }

                onFullScreenRequested: function(request) {
                    request.accept()
                }
            }
        }

        InputPanel {
            id: inputPanel

            z: 100
            width: 850
            x: 0
            y: root.height

            states: State {
                name: "visible"
                when: inputPanel.active

                PropertyChanges {
                    target: inputPanel
                    y: root.height - inputPanel.height
                }
            }

            transitions: Transition {
                NumberAnimation {
                    properties: "y"
                    duration: 120
                    easing.type: Easing.OutQuad
                }
            }
        }
    }
}
