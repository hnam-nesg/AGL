#include <QObject>
#include <QFile>
#include <QByteArray>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

class MeshLoader : public QObject {
    Q_OBJECT
public:
    explicit MeshLoader(QObject *parent = nullptr) : QObject(parent) {}

    // Hàm gọi từ Main Thread
    void loadMultipleMeshes(const QStringList& filePaths) {
        for (const QString& path : filePaths) {
            // Chạy hàm loadMeshFileDướiBackground trên một luồng khác
            QFuture<QByteArray> future = QtConcurrent::run(&MeshLoader::loadMeshFileUnderBackground, path);

            // Dùng Watcher để nhận kết quả khi thread chạy xong
            QFutureWatcher<QByteArray>* watcher = new QFutureWatcher<QByteArray>(this);
            connect(watcher, &QFutureWatcher<QByteArray>::finished, this, [this, watcher, path]() {
                QByteArray meshData = watcher->result();

                // Ở đây bạn nhận được data, có thể truyền vào QQuick3DGeometry
                qDebug() << "Load xong file:" << path << "- Dung lượng:" << meshData.size() << "bytes";

                watcher->deleteLater();
            });
            watcher->setFuture(future);
        }
    }

private:
    // Hàm này chạy ngầm, không làm giật UI
    static QByteArray loadMeshFileUnderBackground(const QString& filePath) {
        QFile file(filePath);
        QByteArray data;

        if (file.open(QIODevice::ReadOnly)) {
            // Sử dụng Memory Mapping để map trực tiếp file vào RAM
            uchar* memory = file.map(0, file.size());
            if (memory) {
                // Đẩy thẳng vào QByteArray (deep copy nếu cần giữ data sau khi unmap)
                data = QByteArray(reinterpret_cast<const char*>(memory), file.size());
                file.unmap(memory); // Giải phóng map
            } else {
                // Fallback nếu OS không hỗ trợ memory mapping
                data = file.readAll();
            }
        }
        return data;
    }
};
