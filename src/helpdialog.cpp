#include "helpdialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

HelpDialog::HelpDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("帮助说明"));
    setMinimumWidth(450);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto *titleLabel = new QLabel(QStringLiteral("<b>AIPlayer 播放器</b>"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: bold; margin-bottom: 10px;"));
    layout->addWidget(titleLabel);

    auto *descLabel = new QLabel(QStringLiteral("这是一款集成了本地视频播放、在线流媒体播放，以及实时 Whisper 语音识别和双语字幕翻译的跨平台播放器。"), this);
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    auto *streamTitle = new QLabel(QStringLiteral("<b>支持的网络流媒体协议：</b>"), this);
    layout->addWidget(streamTitle);

    QString protocols = QStringLiteral(
        "<ul>"
        "<li><b>HTTP / HTTPS</b>: 支持直接播放常见的在线 mp4、mkv 等视频文件。</li>"
        "<li><b>HLS (m3u8)</b>: 苹果推出的流媒体协议，广泛用于在线视频网站点播和直播。</li>"
        "<li><b>RTMP (rtmp://)</b>: 实时消息传输协议，常用于斗鱼、虎牙等直播平台的推流和拉流。</li>"
        "<li><b>RTSP (rtsp://)</b>: 实时流传输协议，广泛应用于局域网 IPC 监控摄像头、安防系统。</li>"
        "<li><b>磁力链接 (magnet:?xt=urn:btih:...)</b>: 基于 WebTorrent 技术的 P2P 边下边播功能。需要本地系统安装有 node 和 webtorrent-cli 依赖。</li>"
        "<li><b>YouTube 等在线视频解析</b>: 如果环境内配置了 yt-dlp/youtube-dl，MPV 内核支持直接解析播放绝大多数在线视频网站的链接。</li>"
        "</ul>"
        "<i>注：目前针对在线网络流，播放器会开启边下边播功能，但暂时会禁用 Whisper 本地 ASR 自动字幕提取。</i>"
    );

    auto *protocolsLabel = new QLabel(protocols, this);
    protocolsLabel->setWordWrap(true);
    protocolsLabel->setTextFormat(Qt::RichText);
    layout->addWidget(protocolsLabel);

    auto *shortcutTitle = new QLabel(QStringLiteral("<b>快捷键说明：</b>"), this);
    layout->addWidget(shortcutTitle);

    QString shortcuts = QStringLiteral(
        "<ul>"
        "<li><b>空格键 (Space)</b>: 播放 / 暂停</li>"
        "<li><b>← / → 方向键</b>: 快退 / 快进 5 秒</li>"
        "<li><b>↑ / ↓ 方向键</b>: 音量增大 / 减小</li>"
        "<li><b>F 键</b>: 进入 / 退出全屏</li>"
        "<li><b>Esc 键</b>: 退出全屏</li>"
        "</ul>"
    );
    auto *shortcutsLabel = new QLabel(shortcuts, this);
    shortcutsLabel->setWordWrap(true);
    shortcutsLabel->setTextFormat(Qt::RichText);
    layout->addWidget(shortcutsLabel);

    layout->addStretch();

    auto *closeButton = new QPushButton(QStringLiteral("关闭"), this);
    connect(closeButton, &QPushButton::clicked, this, &HelpDialog::accept);
    
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeButton);
    btnLayout->addStretch();

    layout->addLayout(btnLayout);
}