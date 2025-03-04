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
    void setupUi();
    void createLayout();
    void setupAppNameLabel();
    void setupProgressBar();
    void setupStatusLabel();
    void setupAnimations();
    void setupProgressAnimation();
    void setupStatusAnimation();
    void setupAppNameAnimation();
    
    QLabel* statusLabel;
    QLabel* appNameLabel;
    QProgressBar* progressBar;
    
    QPropertyAnimation* progressAnimation;
    QPropertyAnimation* statusOpacityAnimation;
    QGraphicsOpacityEffect* statusOpacityEffect;
    QPropertyAnimation* appNameAnimation;
};

#endif
