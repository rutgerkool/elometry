#include "gui/views/LoadingView.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QFont>
#include <QTimer>
#include <QtWidgets/QApplication>
#include <QEasingCurve>

LoadingView::LoadingView(QWidget* parent)
    : QWidget(parent)
    , m_statusLabel(nullptr)
    , m_appNameLabel(nullptr)
    , m_progressBar(nullptr)
    , m_progressAnimation(std::make_unique<QPropertyAnimation>())
    , m_statusOpacityAnimation(std::make_unique<QPropertyAnimation>())
    , m_statusOpacityEffect(std::make_unique<QGraphicsOpacityEffect>())
    , m_appNameAnimation(std::make_unique<QPropertyAnimation>())
{
    setupUi();
    setupAnimations();
}

void LoadingView::setupUi() {
    setupLayout();
    setupAppNameLabel();
    setupProgressBar();
    setupStatusLabel();
    
    setWindowTitle("Loading Elometry");
    setMinimumSize(500, 350);
}

void LoadingView::setupLayout() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    mainLayout->addStretch();
    
    m_appNameLabel = new QLabel("Elometry", this);
    m_progressBar = new QProgressBar(this);
    m_statusLabel = new QLabel("Initializing", this);
    
    mainLayout->addWidget(m_appNameLabel);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_statusLabel);
    
    mainLayout->addStretch();
}

void LoadingView::setupAppNameLabel() {
    QFont headerFont("Segoe UI", 36);
    headerFont.setBold(true);
    m_appNameLabel->setFont(headerFont);
    m_appNameLabel->setStyleSheet("color: #0c7bb3;");
    m_appNameLabel->setAlignment(Qt::AlignCenter);
}

void LoadingView::setupProgressBar() {
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setMinimumWidth(400);
    m_progressBar->setMinimumHeight(10);
    m_progressBar->setMaximumHeight(10);
    m_progressBar->setTextVisible(false);
}

void LoadingView::setupStatusLabel() {
    m_statusOpacityEffect->setOpacity(1.0);
    m_statusLabel->setGraphicsEffect(m_statusOpacityEffect.get());
    
    m_statusLabel->setAlignment(Qt::AlignCenter);
    
    QFont statusFont("Segoe UI", 12);
    m_statusLabel->setFont(statusFont);
    m_statusLabel->setStyleSheet("color: #a0a0a0;");
}

void LoadingView::setupAnimations() {
    setupProgressAnimation();
    setupStatusAnimation();
    setupAppNameAnimation();
}

void LoadingView::setupProgressAnimation() {
    m_progressAnimation->setTargetObject(m_progressBar);
    m_progressAnimation->setPropertyName("value");
    m_progressAnimation->setDuration(300);
    m_progressAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void LoadingView::setupStatusAnimation() {
    m_statusOpacityAnimation->setTargetObject(m_statusOpacityEffect.get());
    m_statusOpacityAnimation->setPropertyName("opacity");
    m_statusOpacityAnimation->setDuration(250);
    m_statusOpacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

void LoadingView::setupAppNameAnimation() {
    m_appNameAnimation->setTargetObject(m_appNameLabel);
    m_appNameAnimation->setPropertyName("geometry");
    m_appNameAnimation->setDuration(800);
    m_appNameAnimation->setEasingCurve(QEasingCurve::OutBack);
}

void LoadingView::updateStatus(const QString& status) {
    m_statusOpacityAnimation->setStartValue(1.0);
    m_statusOpacityAnimation->setEndValue(0.0);
    
    disconnect(m_statusOpacityAnimation.get(), &QPropertyAnimation::finished, this, nullptr);
    
    connect(m_statusOpacityAnimation.get(), &QPropertyAnimation::finished, this, [this, status]() {
        m_statusLabel->setText(status);
        m_statusOpacityAnimation->setStartValue(0.0);
        m_statusOpacityAnimation->setEndValue(1.0);
        m_statusOpacityAnimation->start();
        disconnect(m_statusOpacityAnimation.get(), &QPropertyAnimation::finished, this, nullptr);
    });
    
    m_statusOpacityAnimation->start();
    QApplication::processEvents();
}

void LoadingView::updateProgress(int value) {
    m_progressAnimation->stop();
    m_progressAnimation->setStartValue(m_progressBar->value());
    m_progressAnimation->setEndValue(value);
    m_progressAnimation->start();
    QApplication::processEvents();
}

void LoadingView::markLoadingComplete() {
    QRect currentGeometry = m_appNameLabel->geometry();
    m_appNameAnimation->setStartValue(currentGeometry);
    
    QRect endGeometry = currentGeometry;
    endGeometry.translate(0, -10);
    
    m_appNameAnimation->setEndValue(endGeometry);
    m_appNameAnimation->start();
    
    updateStatus("Loading complete!");
    
    QTimer::singleShot(1000, this, &LoadingView::loadingFinished);
}
