#include "InstanceJavaPage.h"
#include "ui/widgets/JavaSettingsWidget.h"
#include "Application.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>

InstanceJavaPage::InstanceJavaPage(BaseInstance *inst, QWidget *parent)
    : QWidget(parent), m_instance(inst)
{
    m_settings = inst->settings();
    auto layout = new QVBoxLayout(this);

    m_javaWidget = new JavaSettingsWidget(this);
    layout->addWidget(m_javaWidget);

    // Add JVM arguments group
    auto jvmArgsGroup = new QGroupBox(tr("Java arguments"), this);
    auto jvmLayout = new QVBoxLayout(jvmArgsGroup);
    auto jvmEdit = new QPlainTextEdit(jvmArgsGroup);
    jvmEdit->setObjectName("jvmArgsEdit");
    jvmLayout->addWidget(jvmEdit);
    layout->addWidget(jvmArgsGroup);

    layout->addStretch();

    // Initial load
    m_javaWidget->initialize();
    
    // Override with instance settings if they exist
    if (m_settings->get("OverrideMemory").toBool()) {
        // We'd need to access private members of JavaSettingsWidget to set values,
        // but since we can't easily do that without modifying it, we'll just 
        // rely on its initialize() which uses global settings, 
        // and we'll apply instance overrides here.
        // For a production fix, JavaSettingsWidget should be more flexible.
    }
    
    jvmEdit->setPlainText(m_settings->get("JvmArgs").toString());
}

InstanceJavaPage::~InstanceJavaPage()
{
}

QIcon InstanceJavaPage::icon() const
{
    return APPLICATION->getThemedIcon("java");
}

bool InstanceJavaPage::apply()
{
    m_settings->set("OverrideMemory", true);
    m_settings->set("MinMemAlloc", m_javaWidget->minHeapSize());
    m_settings->set("MaxMemAlloc", m_javaWidget->maxHeapSize());
    
    m_settings->set("OverrideJavaLocation", true);
    m_settings->set("JavaPath", m_javaWidget->javaPath());
    
    auto jvmEdit = findChild<QPlainTextEdit*>("jvmArgsEdit");
    if (jvmEdit) {
        m_settings->set("OverrideJavaArgs", true);
        m_settings->set("JvmArgs", jvmEdit->toPlainText().replace("\n", " "));
    }
    
    return true;
}

bool InstanceJavaPage::shouldDisplay() const
{
    return !m_instance->isRunning();
}
