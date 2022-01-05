#ifndef FOCUSABLEOBJECT_H
#define FOCUSABLEOBJECT_H

#include <QObject>
#include <memory>

#include "3rd_party/oxygine-framework/oxygine-framework.h"

class FocusableObject;
using spFocusableObject = oxygine::intrusive_ptr<FocusableObject>;

class FocusableObject : public QObject, public oxygine::Sprite
{
    Q_OBJECT
public:
    explicit FocusableObject();
    virtual ~FocusableObject();

    bool getFocused() const;
    static void looseFocus();
    bool getSubComponent() const;
    void setSubComponent(bool subComponent);
    static FocusableObject* getFocusedObject()
    {
        return m_focusedObject;
    }
    static bool handleEvent(QEvent *event);
    static void handleInputMethodQuery(Qt::InputMethodQuery query, QVariant arg);
protected:
    virtual void focused(){};
    virtual void looseFocusInternal();
    virtual void inputMethodQuery(Qt::InputMethodQuery query, QVariant arg){};
    /**
     * @brief keyInputMethodQueryEvent called in case a focused object shows a virtual key board
     * @param event
     */
    virtual bool doHandleEvent(QEvent *event)
    {
        return false;
    };
signals:
    void sigFocusedLost();
    void sigFocused();
public slots:
    virtual void focusedLost(){};
private slots:
    void focusedInternal();
protected:
    bool m_focused{false};
private:
    static FocusableObject* m_focusedObject;
    static bool m_registeredAtStage;
    bool m_subComponent{false};
};

#endif // FOCUSABLEOBJECT_H
