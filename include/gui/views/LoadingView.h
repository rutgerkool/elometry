#ifndef LOADINGVIEW_H
#define LOADINGVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QProgressBar>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>

class LoadingView : public QWidget {
    Q_OBJECT

public:
    explicit LoadingView(QWidget *parent = nullptr);

public slots:
    void updateStatus(const QString& status);
    void updateProgress(int value);
    void markLoadingComplete();
    
signals:
    void loadingFinished();

private:
    QLabel* statusLabel;
    QLabel* appNameLabel;
    QProgressBar* progressBar;
    
    QPropertyAnimation* progressAnimation;
    QPropertyAnimation* statusOpacityAnimation;
    QGraphicsOpacityEffect* statusOpacityEffect;
    QPropertyAnimation* appNameAnimation;
    
    void setupUi();
    void setupAnimations();
};

#endif
