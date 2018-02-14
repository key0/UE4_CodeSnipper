#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QApplication>

#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QPropertyAnimation>

#include <QClipboard>
#include <QDir>

template <class T, int   N>
int arraySize(  T (&arr)[N] ) {

   return N;

}

struct _Settings {

   QString ue4Path;
   QString projectPath;
   QString projectName;
   QString entityName;

   _Settings() : ue4Path     ( "Ue4Path"     )
               , projectPath ( "ProjectPath" )
               , projectName ( "ProjectName" )
               , entityName  ( "EntityName"  ) {}
};

class Widget : public QWidget {

    Q_OBJECT

public:
    explicit Widget(QWidget* parent = 0);
    ~Widget();

    _Settings* sets = nullptr ;

protected:

    virtual bool eventFilter(QObject* , QEvent* ) override;
   //void mouseMoveEvent( QMouseEvent* event );
   //void mousePressEvent( QMouseEvent* e );
   //void mouseReleaseEvent( QMouseEvent* e );
protected slots:

    void setOnTop() ;
    void settings() ;
    void on_menu_clicked( QListWidgetItem* item ) ;
    void on_edit_pressed() ;

private:

    QPushButton* btnOntop; // fix main widghet on screen (no animate out)
    QListWidget* lw; // main widget for Menu
    QLineEdit*   le; // edit Names for ue4 entitues to be generated

    QRect myGeometryMax;
    QRect myGeometryMin;
    QPropertyAnimation* anim ; //= new QPropertyAnimation( this , "geometry" );
    QClipboard* clip = nullptr;
    Qt::WindowFlags defWindow ;
    bool isOnTop = false ;
    //bool isSimpleMode = true ;
    void createBaseMenu ( QListWidget* lw );

    static const QStringList menuBase() {

        QStringList strings = { "[0] Create .."
                               ,"[1] C::CreateDefSubobject<>"
                               ,"[2] C::FObjectFinder<>"
                               ,"[3] NewObject<>"
                               ,"[4] LoadObject<>"   };
        return strings;
    }

    static const QStringList menuBaseCombo() {

        QStringList strings = { "COMBO:[1]+[2](SM/SK only)"
                               ,"COMBO:[3]+[4](SM/SK only)"
                               ,"!_Del_! Dirs : Bin,Inter,Saved"};
        return strings;
    }

    static const QStringList menuBaseTrnasform() {

        QStringList strings = { "FVector(...)"
                               ,"FRotator(...)"  };
        return strings;
    }

    static const QStringList menuCreate() {

        QStringList strings = { ".."
                               ,"..Gen Qt Project"
                               ,"..New Game Mode"
                               ,"..New PlayerController"
                               ,"..New AIController"
                               ,"..New Actor"
                               ,"..New Pawn"
                               ,"..New Character"
                               ,"..New Hud" };
        return strings;
    }

    static const QStringList menudDefaultSubobject() {

        QStringList strings = { ".."
                               ,"..SceneComponent"
                               ,"..SphereComponent"
                               ,"..BoxComponent"
                               ,"..CapsuleComponent"
                               ,"..StaticMeshComponent"
                               ,"..SpotLightComponent" };


        return strings;
    }

    static const QStringList menuNewObject() {

        QStringList strings = { ".."
                               ,"..SceneComponent "// hack - added " " to diff defaultSubobjectMenu
                               ,"..SphereComponent "
                               ,"..BoxComponent "
                               ,"..CapsuleComponent "
                               ,"..StaticMeshComponent "
                               ,"..SpotLightComponent " };

        return strings;
    }

    enum e_Entity {

        GameMode
       ,PC
       ,AIC
       ,Actor
       ,Pawn
       ,Char
       ,Hud

    };

    e_Entity entity;
};

namespace Create {

    void QtProject      ( _Settings* sets ) ;
    void newGameMode    ( _Settings* sets ) ;
    void newPC          ( _Settings* sets ) ;
    void newAIC         ( _Settings* sets ) ;
    void newActor       ( _Settings* sets ) ;
    void newPawn        ( _Settings* sets ) ;
    void newCharacter   ( _Settings* sets ) ;
    void newHud         ( _Settings* sets ) ;

}

// Sleep
//#include <QThread>
//class Thread: public QThread {
//    public:
//        static void msleep(unsigned long ms) {
//            QThread::msleep(ms);
//        }
//};

namespace win32API {

    //#define IDE_IMPRESS "- Microsoft Visual Studio"
    #define IDE_IMPRESS "- Qt Creator"

    #include "Windows.h"

    int CALLBACK trySetIDE_Active(HWND hWnd, LPARAM lParam = 0) ;
    WINUSERAPI WINBOOL WINAPI EnumWindows(WNDENUMPROC lpEnumFunc,LPARAM lParam = 0);

    // void m_sleep( DWORD msec );
}

#endif // WIDGET_H


