#ifndef LOADINGVIEW_H
#define LOADINGVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QString>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <memory>

class LoadingView final : public QWidget {
    Q_OBJECT

    public:
        explicit LoadingView(QWidget* parent = nullptr);
        ~LoadingView() override = default;

        LoadingView(const LoadingView&) = delete;
        LoadingView& operator=(const LoadingView&) = delete;
        LoadingView(LoadingView&&) = delete;
        LoadingView& operator=(LoadingView&&) = delete;

    public slots:
        void updateStatus(const QString& status);
        void updateProgress(int value);
        void markLoadingComplete();
            
    signals:
        void loadingFinished();

    private:
        void setupUi();
        void setupLayout();
        void setupAppNameLabel();
        void setupProgressBar();
        void setupStatusLabel();
        void setupAnimations();
        void setupProgressAnimation();
        void setupStatusAnimation();
        void setupAppNameAnimation();
        
        QLabel* m_statusLabel{nullptr};
        QLabel* m_appNameLabel{nullptr};
        QProgressBar* m_progressBar{nullptr};
        
        std::unique_ptr<QPropertyAnimation> m_progressAnimation;
        std::unique_ptr<QPropertyAnimation> m_statusOpacityAnimation;
        std::unique_ptr<QGraphicsOpacityEffect> m_statusOpacityEffect;
        std::unique_ptr<QPropertyAnimation> m_appNameAnimation;
};

#endif
