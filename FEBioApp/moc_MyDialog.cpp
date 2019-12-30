/****************************************************************************
** Meta object code from reading C++ file 'MyDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MyDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MyDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CActionButton_t {
    QByteArrayData data[6];
    char stringdata0[47];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CActionButton_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CActionButton_t qt_meta_stringdata_CActionButton = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CActionButton"
QT_MOC_LITERAL(1, 14, 7), // "runCode"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 8), // "QString&"
QT_MOC_LITERAL(4, 32, 4), // "code"
QT_MOC_LITERAL(5, 37, 9) // "onClicked"

    },
    "CActionButton\0runCode\0\0QString&\0code\0"
    "onClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CActionButton[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   27,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void CActionButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CActionButton *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->runCode((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->onClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CActionButton::*)(QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CActionButton::runCode)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CActionButton::staticMetaObject = { {
    &QPushButton::staticMetaObject,
    qt_meta_stringdata_CActionButton.data,
    qt_meta_data_CActionButton,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CActionButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CActionButton::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CActionButton.stringdata0))
        return static_cast<void*>(this);
    return QPushButton::qt_metacast(_clname);
}

int CActionButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void CActionButton::runCode(QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_MyDialog_t {
    QByteArrayData data[19];
    char stringdata0[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyDialog_t qt_meta_stringdata_MyDialog = {
    {
QT_MOC_LITERAL(0, 0, 8), // "MyDialog"
QT_MOC_LITERAL(1, 9, 8), // "ResetDlg"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 8), // "RunModel"
QT_MOC_LITERAL(4, 28, 10), // "modelIndex"
QT_MOC_LITERAL(5, 39, 7), // "RunTask"
QT_MOC_LITERAL(6, 47, 4), // "Stop"
QT_MOC_LITERAL(7, 52, 4), // "Quit"
QT_MOC_LITERAL(8, 57, 5), // "Pause"
QT_MOC_LITERAL(9, 63, 8), // "Continue"
QT_MOC_LITERAL(10, 72, 8), // "doAction"
QT_MOC_LITERAL(11, 81, 2), // "id"
QT_MOC_LITERAL(12, 84, 7), // "naction"
QT_MOC_LITERAL(13, 92, 7), // "RunCode"
QT_MOC_LITERAL(14, 100, 8), // "QString&"
QT_MOC_LITERAL(15, 109, 4), // "code"
QT_MOC_LITERAL(16, 114, 12), // "on_modelInit"
QT_MOC_LITERAL(17, 127, 5), // "index"
QT_MOC_LITERAL(18, 133, 15) // "on_timeStepDone"

    },
    "MyDialog\0ResetDlg\0\0RunModel\0modelIndex\0"
    "RunTask\0Stop\0Quit\0Pause\0Continue\0"
    "doAction\0id\0naction\0RunCode\0QString&\0"
    "code\0on_modelInit\0index\0on_timeStepDone"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x0a /* Public */,
       3,    1,   70,    2, 0x0a /* Public */,
       5,    0,   73,    2, 0x0a /* Public */,
       6,    1,   74,    2, 0x0a /* Public */,
       7,    0,   77,    2, 0x0a /* Public */,
       8,    1,   78,    2, 0x0a /* Public */,
       9,    1,   81,    2, 0x0a /* Public */,
      10,    2,   84,    2, 0x0a /* Public */,
      13,    1,   89,    2, 0x0a /* Public */,
      16,    1,   92,    2, 0x0a /* Public */,
      18,    1,   95,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,

       0        // eod
};

void MyDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MyDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ResetDlg(); break;
        case 1: _t->RunModel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->RunTask(); break;
        case 3: _t->Stop((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->Quit(); break;
        case 5: _t->Pause((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->Continue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->doAction((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->RunCode((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->on_modelInit((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->on_timeStepDone((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MyDialog::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_MyDialog.data,
    qt_meta_data_MyDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MyDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
