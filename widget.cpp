#include "widget.h"

#include <QDebug>

#include <QDesktopWidget>
#include <QFile>
//#include <QResizeEvent>

#include <QMimeData>
#include <QFileDialog>
#include <QLineEdit>
#include <QSplitter>

//#include <QMainWindow>
//#include <QLibrary>
#include <QMessageBox>
#include <QSettings>

// ---------- util funcs
void deleteDir( QDir dir ); // delete dir and files recursive
void readVS14Env( QString& vs14Path , QString& wkitsPath ); // read from system where VS and KIT

//QString listFilesDirTree( QStringList fileExt , QDir dir ); // get string of full pathnames in dir whitch given ext
// __________

Widget::Widget(QWidget* parent) :
    QWidget( parent , Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint  ) {

    defWindow = this ->windowFlags() ; // TODO may be not need

//_____try set dark style ..

    QFile fileColorStyle(":qdarkstyle/style.qss");
    if (  fileColorStyle.exists() ) {

        qDebug() << "qdarkstyle exist" ;// QRect(0,0 1280x976)

        fileColorStyle.open( QFile::ReadOnly | QFile::Text );
        QTextStream stream( &fileColorStyle );
        this ->setStyleSheet( stream.readAll() );

        if ( fileColorStyle.isOpen() )
             fileColorStyle.close();
    }

//_____get desktop resolution and set desireable geometry size to widget ( min - max)
    auto screenGeometry = QApplication::desktop() ->availableGeometry();//->availableGeometry();
    this ->setGeometry(  screenGeometry.width()  -2
                       , screenGeometry.height() -400
                       , 2
                       , 400
    );

    myGeometryMin = this ->geometry();
    myGeometryMax = myGeometryMin ;
    myGeometryMax.setLeft( screenGeometry.width() -200 );

//____ set animation property  to widget
    anim = new QPropertyAnimation( this , "geometry" ); // geometry - property of widget
    anim ->setDuration( 300 ); // msec

//_____set widget attribute to chek hover
    //this ->setMouseTracking( true );
    this ->setAttribute( Qt::WA_Hover );
    this ->installEventFilter( this );

    auto btnSet = new QPushButton( this );
    btnSet ->setText("settings");

    auto btnExit = new QPushButton( this );
    btnExit ->setText("x");
        //btnExit ->setGeometry( 0,0,20,20 );
        //btnExit ->setMaximumWidth( 20 );

    btnOntop = new QPushButton( this );
    btnOntop ->setText("+");

    lw = new QListWidget( this );

    createBaseMenu( lw );

    le = new QLineEdit( this ); // for enter Ue4Components type
    le ->hide();

    QGridLayout* layout = new QGridLayout ( this );

    layout ->setAlignment( Qt::AlignTop );
    layout ->setMargin( 4 );

    layout ->addWidget( btnSet,   0, 0       );
    layout ->addWidget( btnOntop, 0, 1       );
    layout ->addWidget( btnExit,  0, 2       );
    layout ->addWidget( lw,       1, 0, 1, 3 );

    //layout ->addWidget( le,       2, 1, 1, 3);

    if( ( clip = QApplication::clipboard() ) == nullptr )
        qDebug() << "QApplication::clipboard() is NULL" ;

    sets = new _Settings();

        qDebug() << "default sets : " << sets->projectName  << sets->projectPath ;

    connect( btnSet      , SIGNAL( clicked()     ),
             this        , SLOT  ( settings()  ) );

    connect( btnOntop    , SIGNAL( clicked()     ),
             this        , SLOT  ( setOnTop()  ) );

    connect( btnExit     , SIGNAL( clicked()     ),
             qApp        , SLOT  ( quit()      ) );

    connect( lw          , SIGNAL( itemClicked      ( QListWidgetItem* )   ),
             this        , SLOT  ( on_menu_clicked  ( QListWidgetItem* ) ) );

    connect( lw          , SIGNAL( itemDoubleClicked( QListWidgetItem* )   ),
             this        , SLOT  ( on_menu_clicked  ( QListWidgetItem* ) ) );

    connect( le          , SIGNAL( returnPressed()     ),
             this        , SLOT  ( on_edit_pressed() ) );
}

void Widget::createBaseMenu ( QListWidget* lw ) {

    if ( nullptr == lw) return;
    lw ->clear();
    lw ->addItems( menuBase() );
    // split_line0.setFlags(Qt::ItemIsEnabled);    //disables selectionable
    lw ->addItem("_______________________________");
    lw ->item( lw->count()-1 ) ->setBackgroundColor( QColor( Qt::gray ) );

    lw ->addItems( menuBaseCombo() );
   //split_line1.setFlags(Qt::ItemIsEnabled);    //disables selectionable
    lw ->addItem( "_______________________________" );
    lw ->item( lw->count()-1 ) ->setBackgroundColor( QColor( Qt::gray ) );

    lw ->addItems( menuBaseTrnasform() );
    lw ->setSelectionMode( QAbstractItemView::NoSelection );

}

void Widget::settings() {

    qDebug() << "..settings pressed " ;

    auto flags = QFlag( QFileDialog::DontResolveSymlinks );

    auto projectPathName = QFileDialog::getOpenFileName( this
                                                        ,"select project Path"
                                                        ,""
                                                        ,"*.uproject"
                                                        ,0// selected filter
                                                        ,flags);

     //dismember full projectPathName to projectName and projectPath
    //r:\m_mUE10_ 4.14\m_mUE10_.uproject
    auto index0 = projectPathName.lastIndexOf('/');
    auto index1 = projectPathName.lastIndexOf('.');
    sets ->projectName = projectPathName.mid( index0 + 1 , index1-index0-1 );
    projectPathName.truncate( index0 );
    sets ->projectPath = projectPathName ;

    //qDebug()<< sets ->projectName ;
    //qDebug()<< sets ->projectPath ;

    if ( QMessageBox::Yes == QMessageBox::question( this
                                                   ,("")
                                                   ,("Setted :\t projectPath = " + sets ->projectPath + "\n\t projectName = " + sets ->projectName + "\n\nSelect UE4 dir ? (it need to gen new projects..)" )
                                                   , QMessageBox::Yes|QMessageBox::No) ) {


        flags = flags | QFileDialog::ShowDirsOnly ;

        sets ->ue4Path = QFileDialog::getExistingDirectory( this
                                                           ,"select UE4 Engine Path"
                                                           ,""
                                                           ,flags);


        QMessageBox::information( this
                                 ,(" -= OK =- ")
                                 ,("Setted :\t projectPath = " + sets ->projectPath + "\n\t projectName = " + sets ->projectName + "\n\t ue4Path =  " + sets ->ue4Path )
                                 ,MB_OK);

    }

   // qDebug() << listFilesDirTree( QStringList("*.h") , QDir ("r:\\m_u14\\Source") );

}

void Widget::setOnTop() {

    isOnTop = !isOnTop ;

    if ( isOnTop ) {

        this ->setWindowFlags(  this ->windowFlags() | Qt::WindowStaysOnTopHint );//setAttribute( );// Qt::WA_Hover
        this ->show();
        btnOntop ->setText(">>");

    } else {

        //this ->setWindowFlags( defWindow );
        //this ->show();
        anim ->setEndValue( myGeometryMin );
        anim ->start();
        btnOntop ->setText("+");
    }

// qDebug() << "ON TOP SWITH" ;

}

bool Widget::eventFilter(QObject*  object, QEvent*  event) {

        QEvent::Type type = event->type();

        if  ( type == QEvent::HoverEnter ) {

            //qDebug() << "mouse intro " << this ->geometry().width() ;
            lw ->activateWindow();
            lw ->setFocus();
            if ( this ->geometry().width() >= myGeometryMin.width() ) {

                anim ->stop();
                anim ->setEndValue( myGeometryMax );
                anim ->start();

            }

        } else if ( type == QEvent::HoverLeave ) {

            //qDebug() << "mouse leave " << this ->geometry().width();


            if ( ! isOnTop )
            if ( this ->geometry().width() <= myGeometryMax.width() ) {

               anim ->stop();
               anim ->setEndValue( myGeometryMin );
               anim ->start();
               createBaseMenu( lw );
            }

        } else if ( type == QEvent::WindowActivate ) {//QEvent::ActivationChange

             //qDebug() << " WindowActivate !!" ;

             anim ->stop();
             anim ->setEndValue( myGeometryMax );
             anim ->start();

        }

//        else if (type == QEvent::MouseButtonPress) {

//            QMouseEvent *mev = static_cast(event);
//            if (mev) {
//                if (mev->button() == Qt::LeftButton) {
//                    // обработка события щелчка по объекту
//                    ...
//                }
//            }

//        }

    return QWidget::eventFilter( object, event );
}

void Widget::on_menu_clicked( QListWidgetItem* item ) {

    Q_UNUSED(item);

    auto text = lw ->currentItem() ->text() ;
//______ BASE MENU _______
    if ( text == "[0] Create .." ) {

        lw ->clear();
        lw ->addItems( menuCreate() );
        return ;
    }

    if ( text == "[1] C::CreateDefSubobject<>" ) {

        //qDebug() << "C::CreateDefaultSubobject<.> clicked!" ;
        lw ->clear();
        lw ->addItems( menudDefaultSubobject() );
        return ;
    }

    if ( text == "[2] C::FObjectFinder<>" ) {

        QString inClip ="no data in ClipBoard.. ==> ctrl + C on asset in UE4";
     //TODO simple chek clip for useable data
        if( auto data = clip ->mimeData() )
        if(  data ->hasText() ) {

                QString assetName = "";
                QString clipText = data ->text() ;
                clipText.remove("\n");
                QStringList list = clipText.split("'") ;
                qDebug() << list ;
                inClip = "" ;

                for ( int i = 0; i < list.count() - 1 ; i+=2 ) {

                    list.at( i ) == "MaterialInstanceConstant" ?
                       list[ i ] = QString("MaterialInstance") : list.at( i ) ;
                    assetName = list.at( i+1 ).mid( list.at( i+1 ).lastIndexOf('.') + 1 );//"/Game/SM_Cube.SM_Cube" ==> SM_Cube
                    clipText = ( "auto " + list.at( i ) + QString("%1").arg( i/2 ) );
                    clipText+= ( " = ConstructorHelpers::FObjectFinder<U" + list.at( i ));
                    clipText+= ( ">( TEXT(\"" + list.at( i+1 ));
                    clipText+=   "\") ) ;\n" ;
                    clipText+= ( "if ( " + list.at( i ) + QString("%1").arg( i/2 ) );
                    clipText+=   ".Succeeded() )\n" ;
                    clipText+=   "\t auto " ;
                    clipText+= ( assetName + " = " );
                    clipText+= ( list.at( i ) + QString("%1").arg( i/2 ) + ".Object;\n\n" );

                    inClip += clipText;
                   // clipText = "";
                }
        }
        clip ->setText( inClip );
        lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
        win32API::EnumWindows( win32API::trySetIDE_Active );
        return ;
    }

    if ( text == "[3] NewObject<>" ) {

        //qDebug() << "NewObject<.> clicked!" ;
        lw ->clear();
        lw ->addItems( menuNewObject() );

    }

    if ( text == "[4] LoadObject<>" ) {

        //qDebug() << "LoadObject<.> clicked!" ;
        QString inClip ="no data in ClipBoard.. ==> ctrl + C on asset in UE4";

        if( auto data = clip ->mimeData() )
        if(  data ->hasText() ) {

            QString assetName = "";
            QString clipText = data ->text() ;
            clipText.remove("\n");
            QStringList list = clipText.split("'") ;
            qDebug() << list ;
            inClip = "" ;

            for ( int i = 0; i < list.count() - 1 ; i+=2 ) {

                list.at( i ) == "MaterialInstanceConstant" ?
                   list[ i ] = QString("MaterialInstance") : list.at( i ) ;
                assetName = list.at( i+1 ).mid( list.at( i+1 ).lastIndexOf('.') + 1 );//"/Game/SM_Cube.SM_Cube" ==> SM_Cube
                clipText = ( "auto " + list.at( i ) + QString("%1").arg( i/2 ) );
                clipText+= ( " = LoadObject<U" + list.at( i ) );
                clipText+= ( ">( this , TEXT(\"" + list.at( i+1 ));
                clipText+=   "\") ) ;\n\n" ;

                inClip += clipText;
               // clipText = "";
            }

        }

        clip ->setText( inClip );
        lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
        win32API::EnumWindows( win32API::trySetIDE_Active );
        return ;

    }

    if ( text == "COMBO:[1]+[2](SM/SK only)" ) {// SM and SK NOW


        QString inClip ="no data in ClipBoard.. ==> ctrl + C on asset in UE4";

        if( auto data = clip ->mimeData() )
        if(  data ->hasText() ) { //COMBO :

            qDebug() << "..StaticMeshComponent COMBO !!" ;

                    QString assetName = "";
                    QString clipText = data ->text() ;
                    clipText.remove("\n");
                    QStringList list = clipText.split("'") ;
                    //qDebug() << list ;

                    inClip = "" ;

                    for ( int i = 0; i < list.count() - 1 ; i+=2 ) {

                        list.at( i ) == "MaterialInstanceConstant" ?
                           list[ i ] = QString("MaterialInstance") : list.at( i ) ;
                        assetName = list.at( i+1 ).mid( list.at( i+1 ).lastIndexOf('.') + 1 );//"/Game/SM_Cube.SM_Cube" ==> SM_Cube
                        //clipText+= ( "auto " + list.at( i ) );
                        clipText = ( "auto comp" + QString("%1").arg( i/2 ) );
                        clipText+= ( " = CreateDefaultSubobject<U" + list.at( i ));
                        clipText+= ( "Component>(\"" + assetName ) ;
                        clipText+= ( QString("%1").arg( i/2 ) + "\") ;\n" );
                        clipText+= ( "if ( comp" + QString("%1").arg( i/2 ) );
                        clipText+=   " != nullptr ) {\n\n\t";

                        clipText+= ( "auto " + list.at( i ) );
                        clipText+= ( " = ConstructorHelpers::FObjectFinder<U" + list.at( i ) );
                        clipText+= ( ">(TEXT( \"" +  list.at( i+1 ) );
                        clipText+=   "\"));\n\t" ;
                        clipText+= ( "if ( " + list.at( i ) );
                        clipText+= ( ".Succeeded() )\n\t");
                        clipText+= ( "comp" + QString("%1").arg( i/2 ) );

                        list.at( i ) == "StaticMesh" ?
                            clipText+= " ->SetStaticMesh ( StaticMesh.Object );\n\n}\n"        :
                            clipText+= " ->SetSkeletalMesh ( SkeletalMesh.Object );\n\n}\n" ;


                        inClip += clipText;
                       // clipText = "";
                    }

           clip ->setText( inClip );
           lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
           win32API::EnumWindows( win32API::trySetIDE_Active );
        }
        return ;
    }

    if ( text == "COMBO:[3]+[4](SM/SK only)" ) {// SM and SK NOW


        QString inClip ="no data in ClipBoard.. ==> ctrl + C on asset in UE4";

        if( auto data = clip ->mimeData() )
        if(  data ->hasText() ) { //COMBO :

            qDebug() << "[3] + [4] COMBO !!" ;

                    QString assetName = "";
                    QString clipText = data ->text() ;
                    clipText.remove("\n");
                    QStringList list = clipText.split("'") ;
                    //qDebug() << list ;

                    inClip = "" ;

                    for ( int i = 0; i < list.count() - 1 ; i+=2 ) {

                        list.at( i ) == "MaterialInstanceConstant" ?
                           list[ i ] = QString("MaterialInstance") : list.at( i ) ;
                        assetName = list.at( i+1 ).mid( list.at( i+1 ).lastIndexOf('.') + 1 );//"/Game/SM_Cube.SM_Cube" ==> SM_Cube
                        //clipText+= ( "auto " + list.at( i ) );
                        clipText = ( "auto comp" + QString("%1").arg( i/2 ) );
                        clipText+= ( " =  NewObject<U" + list.at( i ));
                        clipText+= ( "Component>( this, TEXT( \"" + assetName ) ;
                        clipText+= ( QString("%1").arg( i/2 ) + "\") ) ;\n" );
                        clipText+= ( "if ( comp" + QString("%1").arg( i/2 ) );
                        clipText+=   " != nullptr ) {\n\n\t";

                        clipText+= ( "auto " + list.at( i ) );
                        clipText+= ( " = LoadObject<U" + list.at( i ) );
                        clipText+= ( ">( this, TEXT( \"" +  list.at( i+1 ) );
                        clipText+=   "\"));\n\t" ;
                        clipText+= ( "if ( " + list.at( i ) );
                        clipText+= ( "!= nullptr )\n\t");
                        clipText+= ( "comp" + QString("%1").arg( i/2 ) );

                        list.at( i ) == "StaticMesh" ?
                            clipText+= " ->SetStaticMesh ( StaticMesh );\n\n}\n"     :
                            clipText+= " ->SetSkeletalMesh ( SkeletalMesh );\n\n}\n" ;

                        //if ( StaticMesh!= nullptr)
                        //     comp0 ->SetStaticMesh( StaticMesh );

                        inClip += clipText;
                    }

           clip ->setText( inClip );
           lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
           win32API::EnumWindows( win32API::trySetIDE_Active );
        }
        return ;
    }

    if ( text == "!_Del_! Dirs : Bin,Inter,Saved" ) {

        QString delited ;
        QString delDirName = sets->projectPath + "/Binaries" ;
        QDir delDir ( delDirName ) ;
        deleteDir( QDir( delDirName ) );
        if ( ! delDir.exists() ) delited += "Bin-OK " ;

        delDirName = sets->projectPath + "/Intermediate" ;
        deleteDir( QDir( delDirName ) );
        delDir.setCurrent( delDirName );
        if ( ! delDir.exists() ) delited += ",Inter-OK " ;

        delDirName = sets->projectPath + "/Saved" ;
        deleteDir( QDir( delDirName ) );
        delDir.setCurrent( delDirName );
        if ( ! delDir.exists() ) delited += ",Saved-OK " ;

        lw ->currentItem() ->setText( delited );
        return;
    }

    if ( text == "FVector(...)" ) {

        if( auto data = clip ->mimeData() ) {
            if( data ->hasText() ) {
                // (X=0.000000,Y=0.000000,Z=0.000000)

                QString clipText = data ->text() ;

                if (    clipText.contains( QChar('=') ) ){ // simple correct clip chek
                     //&& clipText.contains("Y=")
                     //&& clipText.contains("Z=")  ) {

                    QStringList list = clipText.split(",") ;
                    QString inClip = "";
                    clip ->clear();
                    if ( list.count() >= 3 )
                         inClip = (  "FVector(" + list.at(0).mid( list.at(0).indexOf("=")+1)
                                    + ","       + list.at(1).mid( list.at(1).indexOf("=")+1)
                                    + ","       + list.at(2).mid( list.at(2).indexOf("=")+1)  );
                    //qDebug() << list ;
                    clip ->setText( inClip );
                    lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
                    win32API::EnumWindows( win32API::trySetIDE_Active );
                    return;
                }
            }
        }

        clip ->clear();
        clip ->setText( "FVector( 0, 0, 0 )" );
        lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
        win32API::EnumWindows( win32API::trySetIDE_Active );

        qDebug() << "No data in CLIP , created zero FVector()" ;
        return;

    }

    if ( text == "FRotator(...)" ) {

        if( auto data = clip ->mimeData() ) {
            if( data ->hasText() ) {
                // (X=0.000000,Y=0.000000,Z=0.000000)

                QString clipText = data ->text() ;

                if (    clipText.contains( QChar('=') ) ) { // simple correct clip chek
                    // && clipText.contains("Y=")
                    // && clipText.contains("Z=")  ) {

                    QStringList list = clipText.split(",") ;
                    QString inClip = "";
                    clip ->clear();
                    if ( list.count() >= 3 )
                         inClip = (  "FRotator(" + list.at(0).mid( list.at(0).indexOf("=")+1)
                                    + ","        + list.at(1).mid( list.at(1).indexOf("=")+1)
                                    + ","        + list.at(2).mid( list.at(2).indexOf("=")+1)  );
                    //qDebug() << list ;
                    clip ->setText( inClip );
                    lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
                    win32API::EnumWindows( win32API::trySetIDE_Active );
                    return;
                }
            }
        }

        clip ->clear();
        clip ->setText( "FRotator( 0, 0, 0 )" );
        lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
        win32API::EnumWindows( win32API::trySetIDE_Active );
        return ;
    }
//_____ <Create..> sub menu _____
    if ( text == ".." ) {

        if ( le != nullptr ) le ->hide();
        createBaseMenu( lw );
        return ;// return to base menu

    }

    if ( text == "..Gen Qt Project" ) {

        Create::QtProject( sets );
        return ;
    }

    if ( text == "..New Game Mode" ) {

        entity = GameMode ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }

    if ( text == "..New PlayerController" ) {

        entity = PC ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }

    if ( text == "..New AIController" ) {

        entity = AIC ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }
    if ( text == "..New Actor" ) { // TODO DRY
// line Edit near cursor to write new actor name ..
        entity = Actor ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }

    if ( text == "..New Pawn" ) {

        entity = Pawn ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }

    if ( text == "..New Character" ) {

        entity = Char ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }

    if ( text == "..New Hud" ) {

        entity = Hud ;
        auto cursorPos = this ->mapFromGlobal( QCursor::pos() ) ;
        auto width     = le ->geometry().width()  ;
        auto height    = le ->geometry().height() ;
        le ->setGeometry( QRect ( cursorPos.x() , cursorPos.y() , width, height ) );
        le ->setFocus();
        le ->show();
        return ;
    }

//_____ <CreateDefaultSubObject> sub menu _____
        if ( text == "..SceneComponent" ) {

            //qDebug() << "..SceneComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto sceneComp = CreateDefaultSubobject<USceneComponent>(\"Scene\") ;");
            return;

        }

        if ( text == "..SphereComponent" ) {

            //qDebug() << "..SphereComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto sphereComp = CreateDefaultSubobject<USphereComponent>(\"Sphere\") ;");
            return ;
        }

        if ( text == "..BoxComponent" ) {

            //qDebug() << "..BoxComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto boxComp = CreateDefaultSubobject<UBoxComponent>(\"Box\") ;");
            return ;
        }

        if ( text == "..CapsuleComponent" ) {

            //qDebug() << "..CapsuleComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto capsuleComp = CreateDefaultSubobject<UCapsuleComponent>(\"Capsule\") ;");
            return ;
        }

        if ( text == "..StaticMeshComponent" ) {

            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto meshComp = CreateDefaultSubobject<UStaticMeshComponent>(\"Mesh\") ;");
            return ;
        }

        if ( text == "..SpotLightComponent" ) {

            //qDebug() << "..SpotLightComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto spotLightComp = CreateDefaultSubobject<USpotLightComponent>(\"SpotLight\") ;");
            return ;
        }

 //_____ <NewObject> sub menu _____
        if ( text == "..SceneComponent " ) {

            //qDebug() << "..SceneComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto sceneComp = NewObject<USceneComponent>( this,TEXT(\"Scene\") ) ;");
            return ;
        }

        if ( text == "..SphereComponent " ) {

            //qDebug() << "..SphereComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto sphereComp = NewObject<USphereComponent>( this,TEXT(\"Sphere\") ) ;");
            return ;
        }

        if ( text == "..BoxComponent " ) {

            //qDebug() << "..BoxComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto boxComp = NewObject<UBoxComponent>( this,TEXT(\"Box\") ) ;");
            return ;
        }

        if ( text == "..CapsuleComponent " ) {

            //qDebug() << "..CapsuleComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto capsuleComp = NewObject<UCapsuleComponent>( this,TEXT(\"Capsule\") ) ;");
            return ;
        }

        if ( text == "..StaticMeshComponent " ) {

            //qDebug() << "..StaticMeshComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto meshComp = NewObject<UStaticMeshComponent>( this,TEXT(\"Mesh\") ) ;");
            return ;
        }

        if ( text == "..SpotLightComponent " ) {

            //qDebug() << "..SpotLightComponent clicked!" ;
            lw ->currentItem() ->setText( "=>>>GENERATED IN CLIP !" );
            clip ->setText("auto spotLightComp = NewObject<USpotLightComponent>( this,TEXT(\"SpotLight\") ) ;");
            return ;
        }

}

void Widget::on_edit_pressed() {

    //qDebug() << "edit pressed!" ;
    sets ->entityName = le ->text();
    le ->hide();

    switch ( entity ) {

        case  GameMode  :

              Create::newGameMode( sets );
              return;

        case  PC  :

              Create::newPC( sets );
              return;

        case  AIC  :

              Create::newAIC( sets );
              return;

        case  Actor :

              Create::newActor( sets );
              return;

        case  Pawn  :

              Create::newPawn( sets );
              return;

        case  Char  :

              Create::newCharacter( sets );
              return;

        case  Hud  :

              Create::newHud( sets );
              return;
    }
}

void Create::QtProject( _Settings* sets ) {

    qDebug() << "..QtProject to create! " ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4Qtproject.txt" );

    if( file.open(QIODevice::ReadOnly | QIODevice::Text ) ) {

       if ( ! QDir( sets->projectPath + "/" + "QtProject" ).exists() )
              QDir().mkdir(QString ( sets->projectPath + "/" + "QtProject" ));

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();
 //________GEN BUILD.BAT ________________________________
            if ( in_string.contains("//BUILD_FILE") ) {

                qDebug()<< "BUILD_FILE PART FOUND !!!";

                QFile fileBuildBat( sets->projectPath + "/" + "build_" + sets->projectName + ".bat"  );
                if( fileBuildBat.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileBuildBat );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "pathUE4"     , sets->ue4Path     );
                        in_string.replace ( "nameProject" , sets->projectName );
                        in_string.replace ( "pathProject" , sets->projectPath );

                        if ( in_string.contains( "//RUN_FILE") ) {

                            qDebug()<< "RUN_FILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileBuildBat.exists() )
                         fileBuildBat.close();
                }//fileBuild open
            }//<< BUILD

 //________GEN RUN.BAT______________________________
            if ( in_string.contains("//RUN_FILE") ) {

                QFile fileRunBat( sets->projectPath + "/" + "run_" + sets->projectName + ".bat"  );
                if( fileRunBat.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileRunBat );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "pathUE4"     , sets->ue4Path );
                        in_string.replace ( "nameProject" , sets->projectName );
                        in_string.replace ( "pathProject" , sets->projectPath );

                        if ( in_string.contains( "//DEFINES_FILE") ) {

                            qDebug()<< "DEFINES_FILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileRunBat.exists() )
                         fileRunBat.close () ;
                }//<fileRunBat open
            }//<<RUN

//________GEN DEFINES_______________________________
            if ( in_string.contains("//DEFINES_FILE") ) {

                QFile fileDefines( sets->projectPath + "//QtProject//defines.pri"  );
                if( fileDefines.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileDefines );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();

                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );


                        if ( in_string.contains( "//INCLUDES_FILE") ) {

                            qDebug()<< "INCLUDES_FILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileDefines.exists() )
                        fileDefines.close () ;
                }//<fileDefines open
            }//<<DEFINES

//________GEN INCLUDES______________________________
            if ( in_string.contains("//INCLUDES_FILE") ) {

                QFile fileIncludes( sets->projectPath + "//QtProject//includes.pri"  );
                if( fileIncludes.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileIncludes );
                    QString      vs14Path={} , wkitsPath={}  ;
                    readVS14Env( vs14Path , wkitsPath );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "pathVS"      , vs14Path          ) ;
                        in_string.replace ( "pathWKIT"    , wkitsPath         ) ;
                        in_string.replace ( "nameProject" , sets->projectName ) ;
                        in_string.replace ( "pathUE4"     , sets->ue4Path     ) ;

                        if ( in_string.contains( "//PRO_FILE") ) {

                            qDebug()<< "PRO_FILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileIncludes.exists() )
                         fileIncludes.close () ;
                }//<<fileIncludes open
            }//<<INCLUDES

//________GEN PRO ______________________________
            if ( in_string.contains("//PRO_FILE") ) {

                QFile filePRO( sets->projectPath + "/QtProject/" + sets->projectName + ".pro" );
                if( filePRO.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &filePRO );
                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        //if (in_string.contains("HEADERS"))
                        //    in_string.append( listFilesDirTree( QStringList("*.h")   ,QDir( sets->projectPath ) ) );
                        //if (in_string.contains("SOURCES"))
                        //    in_string.append( listFilesDirTree( QStringList("*.cpp") ,QDir( sets->projectPath ) ) );
                        in_string.replace ( "nameProject" , sets->projectName ) ;

                        stream << in_string;
                    }
                    if ( filePRO.exists() )
                         filePRO.close () ;
                }//<fileIncludes open
            }//<<INCLUDES
        }//read ue4Qtproject.txt file.atEnd()
     }//open ue4Qtproject.txt
}

void Create::newGameMode( _Settings* sets ) {

    qDebug() << "..newGameMode to create! " ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4gamemode.txt" );

    if( file.open(QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "nameGameMode" ,   sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }
            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

            //________GEN CPP_______________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "nameGameMode", sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }
                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;
    } else {

        qDebug()<< " /templates/ue4gamemode.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

void Create::newPC( _Settings* sets ) {

    qDebug() << "..newPlayerController to create! " ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4PC.txt" );

    if( file.open(QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "namePC" ,      sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }
            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

           //________GEN CPP_______________________________


                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "namePC"      , sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }
                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;
    } else {

        qDebug()<< " /templates/ue4PC.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

void Create::newAIC( _Settings* sets ) {

    qDebug() << "..newAIController to create! " ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4AIC.txt" );

    if( file.open(QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "nameAIC" ,     sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }
            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

           //________GEN CPP_______________________________


                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "nameAIC"      , sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }
                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;

    } else {

        qDebug()<< " /templates/ue4AIC.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

void Create::newActor( _Settings* sets ) {

    qDebug() << "..newActor to create .. " ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4actor.txt" );

    if( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "nameActor" ,   sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }
            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

           //________GEN CPP_______________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "nameActor" ,   sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }
                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;
    } else {

        qDebug()<< "/templates/ue4actor.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

void Create::newPawn( _Settings* sets ) {

    qDebug() << "..newPawn to create! , sets:" << sets ->entityName ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4pawn.txt" );

    if( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "namePawn" ,   sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }

            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

           //________GEN CPP_______________________________


                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "namePawn" ,   sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }

                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;
    } else {

        qDebug()<< "/templates/ue4pawn.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

void Create::newCharacter( _Settings* sets ) {

    //qDebug() << "..newCharacter to create! , sets:" << sets ->entityName ;
    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4char.txt" );

    if( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "nameChar" ,   sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }

            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

           //________GEN CPP_______________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "nameChar" ,   sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }

                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;
    } else {

        qDebug()<< " /templates/ue4char.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

void Create::newHud(  _Settings* sets ) {

    qDebug() << "..newHud to create! " << "sets entytyi" << sets ->entityName ;

    QFile file ( QCoreApplication::applicationDirPath()
                + "/templates/ue4hud.txt" );

    if( file.open(QIODevice::ReadOnly | QIODevice::Text ) ) {

        while ( !file.atEnd() ) {

            QString in_string = file.readLine();

            if ( in_string.contains("//HFILE") ) {

                qDebug()<< "HFILE PART FOUND !!!";

            //________GEN H ________________________________

                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".h"              ;

                        qDebug()<< fileName ;

                QFile fileH ( fileName );

                if( fileH.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileH );

                    while (  !file.atEnd() ) {

                        in_string = file.readLine();
                        in_string.replace ( "nameHUD" ,   sets->entityName );
                        in_string.replace ( "nameProject" , sets->projectName.toUpper() );

                        if ( in_string.contains( "//CPPFILE") ) {

                            qDebug()<< "CPPFILE PART FOUND!" ;
                            break;
                        }

                        stream << in_string;
                    }
                    if ( fileH.exists() )
                         fileH.close();
                }

            }//<<HFILE

            if ( in_string.contains("//CPPFILE") ) {

           //________GEN CPP_______________________________


                QString fileName ( sets->projectPath );
                        fileName += "/Source/"        ;
                        fileName += sets->projectName ;
                        fileName += "/"               ;
                        fileName += sets->entityName  ;//lineEdit->text();
                        fileName += ".cpp"            ;

                qDebug()<< fileName ;

                QFile fileCPP ( fileName );

                if( fileCPP.open( QIODevice::WriteOnly ) ) {

                    QTextStream stream ( &fileCPP );

                    while (  !file.atEnd() ) {

                        QString in_string = file.readLine();
                                in_string.replace ( "nameHUD" ,   sets->entityName  );
                                in_string.replace ( "nameProject" , sets->projectName );

                        stream << in_string;
                    }
                }

                if ( fileCPP.exists() )
                     fileCPP.close () ;
            }//<<CPPFILE"
        } // while template file at end
        if ( file.isOpen() ) file.close() ;

    } else {

        qDebug()<< " /templates/ue4hud.txt not found in " << QCoreApplication::applicationDirPath() ;

    }
}

Widget::~Widget() {

    if ( clip != nullptr ) {

         clip ->clear();
         clip ->disconnect() ;

    }

    delete sets ;
}

int CALLBACK win32API::trySetIDE_Active(HWND hwnd, LPARAM lParam ) {

    Q_UNUSED(lParam);

    WCHAR title[ 255 ];

    if( GetWindowText( hwnd, title, 255 ) ) {
        QString s = QString::fromWCharArray( title ) ;
        if ( s.contains( IDE_IMPRESS ) ) {
            //if ( s.contains( IDE_Qt_IMPRESS ) ) {
            qDebug() << s;

            ShowWindow( hwnd, SW_SHOWMAXIMIZED );
            SetForegroundWindow( hwnd );
                //UpdateWindow( hwnd );
                //SetWindowPos(hwnd,HWND_TOP,0,0,300,300,SWP_SHOWWINDOW);
                //SetActiveWindow( hwnd ) ;
        }
    }
    return true;
}

void deleteDir(QDir dir) {

   QStringList listDirs  = dir.entryList( QDir::Dirs
                                         |QDir::AllDirs
                                         |QDir::NoDotAndDotDot
                                         |QDir::NoSymLinks );

   QStringList listFiles = dir.entryList( QDir::Files );

   for ( QString entry : listFiles) {

      QString entryAbsPath = dir.absolutePath() + "/" + entry;
      QFile::remove( entryAbsPath );
   }

   for ( QString entry : listDirs ) {

      QString entryAbsPath = dir.absolutePath() + "/" + entry;
      deleteDir( QDir( entryAbsPath ) );
   }

   QDir().rmdir( dir.absolutePath() );
}

//QString listFilesDirTree( QStringList fileExt , QDir dir ) {// get string of full pathnames in dir whitch given ext

//    static QString resString  = {};
//    static QString currentDir = {};
//    static const QString addPrefix = { "../Source/" };

//    QStringList listDirs  = dir.entryList( QDir::Dirs
//                                          |QDir::AllDirs
//                                          |QDir::NoDotAndDotDot
//                                          |QDir::NoSymLinks );



//    QStringList listFiles = dir.entryList( fileExt , QDir::Files );

//    for ( QString entry : listFiles) {

//       //QString entryAbsPath = dir.absolutePath() + "/" + entry;
//       resString +=  ( addPrefix + currentDir + "/" + entry  + "\\\n" ) ;
//    }

//    for ( QString entry : listDirs ) {

//       QString nextDir = dir.absolutePath() + "/" + entry;
//       currentDir = entry ;
//       listFilesDirTree( fileExt , QDir( nextDir ) );

//    }

//    //resString.truncate( resString.size() );
//    return resString ;
//}

void readVS14Env( QString& vs14Path, QString& wkitsPath ) {

 //_______ Windows Kits ________
    QString volumes = "CDEFGHIJKLMNOPQRSTUVWXYZ";
    wkitsPath = QString(":\\Program Files (x86)\\Windows Kits");
    for ( int i = 0 ; i < volumes.size() ; i++ )
    if  ( QFile::exists( ( volumes[i] + wkitsPath ) ) ) {

        wkitsPath = ( volumes[i] + wkitsPath ) ;
        break;

    }
    //qDebug() << wkitsPath ;
//________ VS 14 ______________
    QSettings settings(  "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VSPerf"
                        ,QSettings::NativeFormat );

    QStringList keyList = settings.allKeys();
    QStringList keyValue;   //  == C:\vs14\Team Tools\Performance Tools\VSPerfReport.exe
                          // need take visual studio install dir like "C:\vs14\"
    for ( auto it : keyList )
    if ( it.compare("VSPerfReportPath") == 0) { // key = VSPerfReportPath

        keyValue = ( settings.value( it ).toString() ).split("\\") ;
        for ( int i = 0 ; i < keyValue.size() ; i++ )
        if ( (keyValue[i]).compare("Team Tools") == 0) {

            vs14Path = ( keyValue[0] + "\\" + keyValue[1] );
            //qDebug() << vs14Path ;
            return;
        }
    }
}


//void Widget::mouseMoveEvent( QMouseEvent* event ) {


////    qDebug() << tr("mouseEvent emitted: [x, y] = [%1, %2]")
////                   .arg(event ->x()).arg(event ->y());

////    if ( ! isAnimate ) {

////        anim ->start();
////        isAnimate = true ;
////        return ;

////    }

//    //QCursor::pos ()
//}
