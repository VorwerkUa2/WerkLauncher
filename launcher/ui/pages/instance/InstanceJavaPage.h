#pragma once

#include <QWidget>
#include "BaseInstance.h"
#include "ui/pages/BasePage.h"
#include <QObjectPtr.h>

class JavaSettingsWidget;

class InstanceJavaPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit InstanceJavaPage(BaseInstance *inst, QWidget *parent = 0);
    virtual ~InstanceJavaPage();
    virtual QString displayName() const override
    {
        return tr("Java");
    }
    virtual QIcon icon() const override;
    virtual QString id() const override
    {
        return "java-settings";
    }
    virtual bool apply() override;
    virtual QString helpPage() const override
    {
        return "Instance-java-settings";
    }
    virtual bool shouldDisplay() const override;

private:
    BaseInstance *m_instance;
    SettingsObjectPtr m_settings;
    JavaSettingsWidget *m_javaWidget;
};
