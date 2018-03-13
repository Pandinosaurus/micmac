/*Header-MicMac-eLiSe-25/06/2007

    MicMac : Multi Image Correspondances par Methodes Automatiques de Correlation
    eLiSe  : ELements of an Image Software Environnement

    www.micmac.ign.fr


    Copyright : Institut Geographique National
    Author : Marc Pierrot Deseilligny
    Contributors : Gregoire Maillet, Didier Boldo.

[1] M. Pierrot-Deseilligny, N. Paparoditis.
    "A multiresolution and optimization-based image matching approach:
    An application to surface reconstruction from SPOT5-HRS stereo imagery."
    In IAPRS vol XXXVI-1/W41 in ISPRS Workshop On Topographic Mapping From Space
    (With Special Emphasis on Small Satellites), Ankara, Turquie, 02-2006.

[2] M. Pierrot-Deseilligny, "MicMac, un lociel de mise en correspondance
    d'images, adapte au contexte geograhique" to appears in
    Bulletin d'information de l'Institut Geographique National, 2007.

Francais :

   MicMac est un logiciel de mise en correspondance d'image adapte
   au contexte de recherche en information geographique. Il s'appuie sur
   la bibliotheque de manipulation d'image eLiSe. Il est distibue sous la
   licences Cecill-B.  Voir en bas de fichier et  http://www.cecill.info.


English :

    MicMac is an open source software specialized in image matching
    for research in geographic information. MicMac is built on the
    eLiSe image library. MicMac is governed by the  "Cecill-B licence".
    See below and http://www.cecill.info.

Header-MicMac-eLiSe-25/06/2007*/

#include "InitOutil.h"
#include "DrawOnMesh.h"
#include "CorrelMesh.h"
#include "Pic.h"
#include "Triangle.h"
#include <stdio.h>
#include "../../uti_phgrm/TiepTri/TiepTri.h"
#include "../../uti_phgrm/TiepTri/MultTieP.h"

// =================== Test Zone ============================
Im2D_REAL4 ImRead(string aNameImTif)
{
   Tiff_Im aTif = Tiff_Im::UnivConvStd(aNameImTif);

   Im2D_REAL4 aI(aTif.sz().x, aTif.sz().y);

   ELISE_COPY
   (
       aI.all_pts(),
       aTif.in(),
       aI.out()
   );
   return aI;
}


void SaveTif(Im2D_REAL4 aIm, string aSaveName)
{

        string aName =  std::string("./") + aSaveName + ".tif";

        L_Arg_Opt_Tiff aL = Tiff_Im::Empty_ARG;
        aL = aL + Arg_Tiff(Tiff_Im::ANoStrip());
        Tiff_Im aRes
                (
                   aName.c_str(),
                   aIm.sz(),
                   GenIm::u_int1,
                   Tiff_Im::No_Compr,
                   Tiff_Im::BlackIsZero,
                   aL
                );
        ELISE_COPY
        (
           aIm.all_pts(),
           aIm.in(),
           aRes.out()
        );

}


void Show(Im2D_REAL4 aIm,Fonc_Num aF, Im2D_REAL4 & aImOut, string aSaveName = "")
{
    ELISE_COPY
    (
       aIm.all_pts(),
       Max(0,Min(255,aF)),
       aImOut.out()
    );
    if (aSaveName != "")
    {
        string aName =  std::string("./") + aSaveName + ".tif";

        L_Arg_Opt_Tiff aL = Tiff_Im::Empty_ARG;
        aL = aL + Arg_Tiff(Tiff_Im::ANoStrip());
        Tiff_Im aRes
                (
                   aName.c_str(),
                   aIm.sz(),
                   GenIm::u_int1,
                   Tiff_Im::No_Compr,
                   Tiff_Im::BlackIsZero,
                   aL
                );
        ELISE_COPY
        (
           aIm.all_pts(),
           Max(0,Min(255,aF)),
           aRes.out()
        );
    }
}

void Show_LAP2(Im2D_REAL4 aIm,Fonc_Num aF, Im2D_REAL4 & aImOut, string aSaveName = "")
{
    ELISE_COPY
    (
       aIm.all_pts(),
       ElAbs(aF),
       aImOut.out()
    );
    if (aSaveName != "")
    {
        string aName =  std::string("./") + aSaveName + ".tif";

        L_Arg_Opt_Tiff aL = Tiff_Im::Empty_ARG;
        aL = aL + Arg_Tiff(Tiff_Im::ANoStrip());
        Tiff_Im aRes
                (
                   aName.c_str(),
                   aIm.sz(),
                   GenIm::u_int1,
                   Tiff_Im::No_Compr,
                   Tiff_Im::BlackIsZero,
                   aL
                );
        ELISE_COPY
        (
           aIm.all_pts(),
           Max(0,Min(255,aF)),
           aRes.out()
        );
    }
}


double Conv1Cell(Im2D_REAL4 & aImgIn, Im2D_REAL8 & aKer, Pt2di & aPos, Pt2di & aSzKer, double & aSomker)
{
    double aSom=0;
    for (int aKx=-aSzKer.x; aKx<=aSzKer.x; aKx++)
    {
        for (int aKy=-aSzKer.y; aKy<=aSzKer.y; aKy++)
        {
            Pt2di aVois(aKx, aKy);
            aSom += aImgIn.GetI(aPos + aVois) * aKer.GetI(aVois + aSzKer);
            //cout<<"Img "<<(aPos + aVois)<<aImgIn.GetI(aPos + aVois)<<" -aKer "<<(aVois + aSzKer)<<aKer.GetI(aVois + aSzKer)<<endl;
        }
    }
    return (aSom/aSomker);
}

double Convol_Withker(Im2D_REAL4 & aImgIn, Im2D_REAL8 & aKer, Im2D_REAL4 & aImgOut)
{
    aImgOut.Resize(aImgIn.sz());
    Pt2di aSzKer(round_up((aKer.sz().x-1)/2), round_up((aKer.sz().y-1)/2));
    Pt2di aRun;

    double aSomKer = aKer.som_rect();
    if (aSomKer == 0)
        aSomKer = 1;
    double Moy = 0;
    int aCnt = 0;

    for (aRun.x = aSzKer.x ;aRun.x < aImgIn.sz().x-aSzKer.x; aRun.x++)
    {
        for (aRun.y = aSzKer.y ;aRun.y < aImgIn.sz().y-aSzKer.y; aRun.y++)
        {
            double aRes = Conv1Cell(aImgIn, aKer, aRun, aSzKer, aSomKer);
            Moy += aRes;
            aCnt++;
            aImgOut.SetI_SVP(aRun, aRes);
        }
    }
    return Moy/aCnt;
}

double Average(Im2D_REAL4 & aImgIn, Pt2di aRab = Pt2di(0,0))
{
    Pt2di aRun;
    double aMoy{0};
    int aCnt{0};
    for (aRun.x = aRab.x; aRun.x < aImgIn.sz().x - aRab.x; aRun.x++)
    {
        for (aRun.y = aRab.y; aRun.y < aImgIn.sz().y - aRab.y; aRun.y++)
        {
           aMoy +=  aImgIn.GetI(aRun);
           aCnt++;
        }
    }
    return (aMoy/aCnt);
}


double Variance(Im2D_REAL4 & aImgIn, double aMoy = 0, Pt2di aRab = Pt2di(0,0))
{
    double Moy=aMoy;
    if (aMoy == 0)
    {
       Moy = aImgIn.moy_rect(Pt2dr(aRab), Pt2dr(aImgIn.sz()-(aRab+Pt2di(1,1))) );
    }
    Pt2di aRun;
    double aSumEcart{0};
    int aCnt{0};
    for (aRun.x = aRab.x; aRun.x < aImgIn.sz().x - aRab.x; aRun.x++)
    {
        for (aRun.y = aRab.y; aRun.y < aImgIn.sz().y - aRab.y; aRun.y++)
        {
           aSumEcart +=  ElSquare(aImgIn.GetI(aRun)-Moy);
           aCnt++;
        }
    }
    return (aSumEcart/aCnt);
}



// ====== Focus measurement operator =====

Im2D_REAL4 Convol_With_ELISE(string aImIn, Im2D_REAL8 & aKer)
{
    Im2D_REAL4 aIm2D = ImRead(aImIn);
    Im2D_REAL4 aIm2D_DNs(aIm2D.sz().x, aIm2D.sz().y);

    Im2D_REAL4 aRes(aIm2D.tx(),aIm2D.ty());
    Fonc_Num aF = aIm2D.in(0);
    double som_Ker = aKer.som_rect();
    if (som_Ker == 0)
        som_Ker=1.0;
   ELISE_COPY(aRes.all_pts(),som_masq(aF, aKer)/som_Ker,aRes.out());
    return aRes;
}

Im2D_REAL4 Convol_With_ELISE(Im2D_REAL4 & aImIn, Im2D_REAL8 & aKer)
{
    Im2D_REAL4 aRes(aImIn.tx(),aImIn.ty());
    Fonc_Num aF = aImIn.in(0);
    double som_Ker = aKer.som_rect();
    if (som_Ker == 0)
        som_Ker=1.0;
    ELISE_COPY(aRes.all_pts(),som_masq(aF, aKer)/som_Ker,aRes.out());
    return aRes;
}

double VarOfLap_LAP4(string aNameIm)
{
    // Variance of Laplacian
    cout<<endl<<" + Im : "<<aNameIm<<endl;
    ElTimer aTimer;
    Im2D_REAL8 aLapl(3,3,
                        "0 1 0 "
                        "1 -4 1 "
                        " 0 1 0"
                   );
    Im2D_REAL8 aDenoise(3,3,
                        "1 1 1 "
                        "1 1 1 "
                        " 1 1 1"
                        );

    Pt2di aSzKer(round_up((aLapl.sz().x-1)/2), round_up((aLapl.sz().y-1)/2));

    Im2D_REAL4 aIm2D = ImRead(aNameIm);
    Im2D_REAL4 aIm2D_DNs(aIm2D.sz().x, aIm2D.sz().y);

    aIm2D_DNs = Convol_With_ELISE(aIm2D, aDenoise);

    Im2D_REAL4 aIm2D_Lpl(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_Lpl = Convol_With_ELISE(aIm2D_DNs, aLapl);
    double aVar = Variance(aIm2D_Lpl, 0, aSzKer);

    return aVar;
}

double VarOfLap_LAP4_G(string aNameIm)
{
    // Variance of Laplacian
    ElTimer aTimer;
    Im2D_REAL8 aLapl(3,3,
                        "0 1 0 "
                        "1 -4 1 "
                        " 0 1 0"
                   );
    Im2D_REAL8 aDenoise(3,3,
                        "1 1 1 "
                        "1 1 1 "
                        " 1 1 1"
                        );
    Pt2di aSzKer(round_up((aLapl.sz().x-1)/2), round_up((aLapl.sz().y-1)/2));
    Im2D_REAL4 aIm2D = ImRead(aNameIm);
    Im2D_REAL4 aIm2D_DNs(aIm2D.sz().x, aIm2D.sz().y);
    Convol_Withker(aIm2D, aDenoise, aIm2D_DNs);
    SaveTif(aIm2D_DNs, aNameIm + "_DnsG");

    Im2D_REAL4 aIm2D_Lpl(aIm2D.sz().x, aIm2D.sz().y);
    Convol_Withker(aIm2D_DNs, aLapl, aIm2D_Lpl);
    double aVar = Variance(aIm2D_Lpl, 0, aSzKer);
    SaveTif(aIm2D_Lpl, aNameIm + "_LplG");


    return aVar;
}

double ModifLap_LAP2 (string aNameIm)
{
    // Modified Laplacian
    ElTimer aTimer;
    Im2D_REAL8 aLapl_x(1,3,
                        "-1 2 -1"
                   );
    Im2D_REAL8 aLapl_y(3,1,
                        "-1 "
                        "2 "
                        "-1"
                   );
    Im2D_REAL8 aDenoise(3,3,
                        "1 1 1 "
                        "1 1 1 "
                        " 1 1 1"
                        );

    Pt2di aSzKer(1,1);


    Im2D_REAL4 aIm2D = ImRead(aNameIm);
    Im2D_REAL4 aIm2D_DNs(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_DNs = Convol_With_ELISE(aIm2D, aDenoise);

    Im2D_REAL4 aIm2D_LplX(aIm2D.sz().x, aIm2D.sz().y);
    Im2D_REAL4 aIm2D_LplY(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_LplX = Convol_With_ELISE(aIm2D_DNs, aLapl_x);
    aIm2D_LplY = Convol_With_ELISE(aIm2D_DNs, aLapl_y);

    Im2D_REAL4 aIm2D_LplSum(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_LplX.bitwise_add(aIm2D_LplY, aIm2D_LplSum);
    double aScore = aIm2D_LplSum.som_rect(Pt2dr(aSzKer), Pt2dr(aIm2D_LplSum.sz() - (aSzKer + Pt2di(1,1))));
    return aScore;
}

double DiagonalLap_LAP3 (string aNameIm)
{
    // Diagonal Laplacian
    ElTimer aTimer;
    Im2D_REAL8 aLapl_x(1,3,
                        "-1 2 -1"
                   );
    Im2D_REAL8 aLapl_y(3,1,
                        "-1 "
                        "2 "
                        "-1"
                   );
    Im2D_REAL8 aLapl_x1(3,3,
                        "0 0 1 "
                        "0 -2 0 "
                        " 1 0 0"
                   );
    Im2D_REAL8 aLapl_x2(3,3,
                        "1 0 0 "
                        "0 -2 0 "
                        " 0 0 1"
                   );
    Im2D_REAL8 aDenoise(3,3,
                        "1 1 1 "
                        "1 1 1 "
                        " 1 1 1"
                        );

    Pt2di aSzKer(1,1);
    double aFac = 1.0/sqrt(2);
    aLapl_x1.multiply(aFac);
    aLapl_x2.multiply(aFac);

    Im2D_REAL4 aIm2D = ImRead(aNameIm);
    Im2D_REAL4 aIm2D_DNs(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_DNs = Convol_With_ELISE(aIm2D, aDenoise);


    Im2D_REAL4 aIm2D_LplX(aIm2D.sz().x, aIm2D.sz().y);
    Im2D_REAL4 aIm2D_LplY(aIm2D.sz().x, aIm2D.sz().y);
    Im2D_REAL4 aIm2D_Lplx1(aIm2D.sz().x, aIm2D.sz().y);
    Im2D_REAL4 aIm2D_Lplx2(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_LplX = Convol_With_ELISE(aIm2D_DNs, aLapl_x);
    aIm2D_LplY = Convol_With_ELISE(aIm2D_DNs, aLapl_y);
    aIm2D_Lplx1 = Convol_With_ELISE(aIm2D_DNs, aLapl_x1);
    aIm2D_Lplx2 = Convol_With_ELISE(aIm2D_DNs, aLapl_x2);

    Im2D_REAL4 aIm2D_LplSum(aIm2D.sz().x, aIm2D.sz().y);
    aIm2D_LplX.bitwise_add(aIm2D_LplY, aIm2D_LplSum);
    aIm2D_LplSum.bitwise_add(aIm2D_Lplx1, aIm2D_LplSum);
    aIm2D_LplSum.bitwise_add(aIm2D_Lplx2, aIm2D_LplSum);

    double aScore = aIm2D_LplSum.som_rect(Pt2dr(aSzKer), Pt2dr(aIm2D_LplSum.sz() - (aSzKer + Pt2di(1,1))));
    return aScore;
}


// ========================================
int Test_Conv(int argc,char ** argv)
{

    Im2D_REAL8 aDenoise(3,3,
                        "1 1 1 "
                        "1 1 1 "
                        " 1 1 1"
                        );

    string aDir = "./";
    string aPat, aPattern;

    ElInitArgMain
    (
          argc,argv,
          LArgMain()  << EAMC(aPattern, "PatIm",  eSAM_IsPatFile),
          LArgMain()
    );
    SplitDirAndFile(aDir, aPat, aPattern);
    cInterfChantierNameManipulateur * aICNM = cInterfChantierNameManipulateur::BasicAlloc(aDir);
    vector<string>  aSetIm = *(aICNM->Get(aPat));
    vector<Pt2dr> aVPair;
    for (uint aKImg=0; aKImg<aSetIm.size(); aKImg++)
    {
        // ====== test convolution function ======
        string aIm = aSetIm[aKImg];
        double aVar = VarOfLap_LAP4(aIm);
        Pt2dr aPair(double(aKImg), aVar);
        aVPair.push_back(aPair);
    }
    sortDescendPt2drY(aVPair);

    cout<<endl<<"+ Sort by sharpness (higher is sharper) : "<<endl;
    for (uint aK=0; aK<aVPair.size(); aK++)
    {
        cout<<" + "<<aSetIm[int(aVPair[aK].x)]<<" - Var "<<aVPair[aK].y<<endl;
    }
    return 0;
}
// ===================  ============================

void Test_Xml()
{
    cXml_TriAngulationImMaster aTriangulation = StdGetFromSI("Tri0.xml",Xml_TriAngulationImMaster);
    std::cout << "Name master " << aTriangulation.NameMaster() << "\n";
    cXml_Triangle3DForTieP aTri;
    aTri.P1() = Pt3dr(1,1,1);
    aTri.P2() = Pt3dr(1,1,2);
    aTri.P3() = Pt3dr(1,1,3);
    aTri.NumImSec().push_back(1);

    aTriangulation.Tri().push_back(aTri);
    

    aTriangulation.NameSec().push_back("toto.tif");

    MakeFileXML(aTriangulation,"Tri1.xml");
    MakeFileXML(aTriangulation,"Tri1.dmp");

    aTriangulation = StdGetFromSI("Tri1.dmp",Xml_TriAngulationImMaster);

     std::cout << "Nb tri " <<  aTriangulation.Tri().size()  << " UnPt " << aTriangulation.Tri()[1].P2() << "\n";


    exit(EXIT_SUCCESS);
}

void Test_FAST()
{
    Tiff_Im * mPicTiff = new Tiff_Im ( Tiff_Im::StdConvGen("./Test.tif",1,false));
    Pt2di mImgSz = mPicTiff->sz();
    TIm2D<double,double> * mPic_TIm2D = new TIm2D<double,double> (mPicTiff->sz());
    ELISE_COPY(mPic_TIm2D->all_pts(), mPicTiff->in(), mPic_TIm2D->out());
    //Im2D<double,double> * mPic_Im2D = new Im2D<double, double> (mPic_TIm2D->_the_im);

    Im2D_Bits<1> aMasq0  = Im2D_Bits<1>(mImgSz.x,mImgSz.y,1);
    TIm2DBits<1> TaMasq0 = TIm2DBits<1> (aMasq0);

    FastNew *aDec = new FastNew(*mPic_TIm2D , 15 , 3 , TaMasq0);
    cout<<aDec->lstPt().size()<<" pts detected "<<endl;
}

    /******************************************************************************
    The main function.
    ******************************************************************************/
int TestGiang_main(int argc,char ** argv)
{

    //Test_Xml();
    //Test_FAST();

    cout<<"********************************************************"<<endl;
    cout<<"*    TestGiang                                         *"<<endl;
    cout<<"********************************************************"<<endl;
        cout<<"dParam : param of detector : "<<endl;
        cout<<"     [FAST_Threshold]"<<endl;
        cout<<"     NO"<<endl;

        string pathPlyFileS ;
        string aTypeD="HOMOLINIT";
        string aFullPattern, aOriInput;
        string aHomolOut = "_Filtered";
        bool assum1er=false;
        int SzPtCorr = 1;int indTri=-1;double corl_seuil_glob = 0.8;bool Test=false;
        int SzAreaCorr = 5; double corl_seuil_pt = 0.9;
        double PasCorr=0.5;
        vector<string> dParam; dParam.push_back("NO");
        bool useExistHomoStruct = false;
        double aAngleF = 90;
        bool debugByClick = false;
        ElInitArgMain
                (
                    argc,argv,
                    //mandatory arguments
                    LArgMain()  << EAMC(aFullPattern, "Pattern of images",  eSAM_IsPatFile)
                    << EAMC(aOriInput, "Input Initial Orientation",  eSAM_IsExistDirOri)
                    << EAMC(pathPlyFileS, "path to mesh(.ply) file - created by Inital Ori", eSAM_IsExistFile),
                    //optional arguments
                    LArgMain()
                    << EAM(corl_seuil_glob, "corl_glob", true, "corellation threshold for imagette global, default = 0.8")
                    << EAM(corl_seuil_pt, "corl_pt", true, "corellation threshold for pt interest, default = 0.9")
                    << EAM(SzPtCorr, "SzPtCorr", true, "1->3*3,2->5*5 size of cor wind for each pt interet  default=1 (3*3)")
                    << EAM(SzAreaCorr, "SzAreaCorr", true, "1->3*3,2->5*5 size of zone autour pt interet for search default=5 (11*11)")
                    << EAM(PasCorr, "PasCorr", true, "step correlation (default = 0.5 pxl)")
                    << EAM(indTri, "indTri", true, "process one triangle")
                    << EAM(assum1er, "assum1er", true, "always use 1er pose as img master, default=0")
                    << EAM(Test, "Test", true, "Test new method - correl by XML")
                    << EAM(aTypeD, "aTypeD", true, "FAST, DIGEO, HOMOLINIT - default = HOMOLINIT")
                    << EAM(dParam,"dParam",true,"[param1, param2, ..] (selon detector - NO if don't have)", eSAM_NoInit)
                    << EAM(aHomolOut, "HomolOut", true, "default = _Filtered")
                    << EAM(useExistHomoStruct, "useExist", true, "use exist homol struct - default = false")
                    << EAM(aAngleF, "angleV", true, "limit view angle - default = 90 (all triangle is viewable)")
                    );

        if (MMVisualMode) return EXIT_SUCCESS;
        vector<double> aParamD = parse_dParam(dParam); //need to to on arg enter
        InitOutil *aChain = new InitOutil(aFullPattern, aOriInput, aTypeD,  aParamD, aHomolOut,
                                          SzPtCorr, SzAreaCorr,
                                          corl_seuil_glob, corl_seuil_pt, false, useExistHomoStruct, PasCorr, assum1er);
        aChain->initAll(pathPlyFileS);
        cout<<endl<<" +++ Verify init: +++"<<endl;
        vector<pic*> PtrPic = aChain->getmPtrListPic();
        for (uint i=0; i<PtrPic.size(); i++)
        {
            cout<<PtrPic[i]->getNameImgInStr()<<" has ";
            vector<PackHomo> packHomoWith = PtrPic[i]->mPackHomoWithAnotherPic;
            cout<<packHomoWith.size()<<" homo packs with another pics"<<endl;
            for (uint j=0; j<packHomoWith.size(); j++)
            {
                if (j!=i)
                    cout<<" ++ "<< PtrPic[j]->getNameImgInStr()<<" "<<packHomoWith[j].aPack.size()<<" pts"<<endl;
            }
        }
        vector<triangle*> PtrTri = aChain->getmPtrListTri();
        cout<<PtrTri.size()<<" tri"<<endl;
        CorrelMesh aCorrel(aChain);
        if (!Test && indTri == -1)
        {
            if (aAngleF == 90)
            {
                cout<<"All Mesh is Viewable"<<endl;
                for (uint i=0; i<PtrTri.size(); i++)
                {
                    if (useExistHomoStruct)
                        aCorrel.correlByCplExist(i);
                    else
                        aCorrel.correlInTri(i);
                }
            }
            else
            {
                cout<<"Use condition angle view"<<endl;
                for (uint i=0; i<PtrTri.size(); i++)
                {
                    if (useExistHomoStruct)
                        aCorrel.correlByCplExistWithViewAngle(i, aAngleF);
                    else
                        aCorrel.correlInTriWithViewAngle(i, aAngleF);
                }
            }
        }
        if (indTri != -1)
        {
            cout<<"Do with tri : "<<indTri<<endl;
            CorrelMesh * aCorrel = new CorrelMesh(aChain);
            if (useExistHomoStruct == false)
                aCorrel->correlInTriWithViewAngle(indTri, aAngleF, debugByClick);
            else
                aCorrel->correlByCplExistWithViewAngle(indTri, aAngleF, debugByClick);
            delete aCorrel;
        }
        if(Test)
        {



        }
        cout<<endl<<"Total "<<aCorrel.countPts<<" cpl NEW & "<<aCorrel.countCplOrg<<" cpl ORG"<<endl;
        cout<<endl;
        return EXIT_SUCCESS;
    }

int IsExtrema(TIm2D<double,double> & anIm,Pt2di aP)
{
    double aValCentr = anIm.get(aP);
    const std::vector<Pt2di> &  aVE = SortedVoisinDisk(0.5,TT_DIST_EXTREMA,true);
    int aCmp0 =0;
    for (int aKP=0 ; aKP<int(aVE.size()) ; aKP++)
    {
        int aCmp = CmpValAndDec(aValCentr,anIm.get(aP+aVE[aKP]),aVE[aKP]);
        if (aKP==0)
        {
            aCmp0 = aCmp;
            if (aCmp0==0) return 0;
        }

        if (aCmp!=aCmp0) return 0;
    }
    return aCmp0;
}

Col_Pal  ColOfType(Video_Win * mW, eTypeTieTri aType)
{
    switch (aType)
    {
          case eTTTMax : return mW->pdisc()(P8COL::red);    //max local => red
          case eTTTMin : return mW->pdisc()(P8COL::blue);   //min local => bleu
          default :;
    }
   return mW->pdisc()(P8COL::yellow);   //No Label => Jaune
}

int TestDetecteur_main(int argc,char ** argv)
{
    Pt3di mSzW;
    string aImg;
    ElInitArgMain
            (
                argc,argv,
                //mandatory arguments
                LArgMain()
                << EAMC(aImg, "img",  eSAM_None)
                << EAMC(mSzW, "mSzW", eSAM_None),
                //optional arguments
                LArgMain()
                );



    if (MMVisualMode) return EXIT_SUCCESS;
    Tiff_Im * mPicTiff = new Tiff_Im ( Tiff_Im::StdConvGen(aImg,1,false));
    Pt2di aSzIm = mPicTiff->sz();
    TIm2D<double,double> mPic_TIm2D(mPicTiff->sz());
    ELISE_COPY(mPic_TIm2D.all_pts(), mPicTiff->in(), mPic_TIm2D.out());
    Im2D<double,double> * anIm = new Im2D<double, double> (mPic_TIm2D._the_im);

    Im2D_Bits<1> aMasq0  = Im2D_Bits<1>(aSzIm.x,aSzIm.y,1);
    TIm2DBits<1> TaMasq0 = TIm2DBits<1> (aMasq0);
    /* video Win */
    Video_Win * mW_Org = 0;
    Video_Win * mW_F = 0;
    Video_Win * mW_FAC = 0; //origin, fast, fast && autocorrel
    Video_Win * mW_Final = 0;

    if (EAMIsInit(&mSzW))
    {
        if (aSzIm.x >= aSzIm.y)
        {
            double scale =  double(aSzIm.x) / double(aSzIm.y) ;
            mSzW.x = mSzW.x;
            mSzW.y = round_ni(mSzW.x/scale);
        }
        else
        {
            double scale = double(aSzIm.y) / double(aSzIm.x);
            mSzW.x = round_ni(mSzW.y/scale);
            mSzW.y = mSzW.y;
        }
        Pt2dr aZ(double(mSzW.x)/double(aSzIm.x) , double(mSzW.y)/double(aSzIm.y) );

        if (mW_Org ==0)
        {
            mW_Org = Video_Win::PtrWStd(Pt2di(mSzW.x*mSzW.z, mSzW.y*mSzW.z), true, aZ*mSzW.z);
            mW_Org->set_sop(Elise_Set_Of_Palette::TheFullPalette());
            mW_Org->set_title((aImg+"_Extr").c_str());
            ELISE_COPY(anIm->all_pts(), anIm->in(), mW_Org->ogray());
        }
        if (mW_F == 0)
        {
            mW_F = Video_Win::PtrWStd(Pt2di(mSzW.x*mSzW.z, mSzW.y*mSzW.z), true, aZ*mSzW.z);
            mW_F->set_sop(Elise_Set_Of_Palette::TheFullPalette());
            mW_F->set_title((aImg+"_FAST").c_str());
            ELISE_COPY(anIm->all_pts(), anIm->in(), mW_F->ogray());
        }
        if (mW_FAC == 0)
        {
            mW_FAC = Video_Win::PtrWStd(Pt2di(mSzW.x*mSzW.z, mSzW.y*mSzW.z), true, aZ*mSzW.z);
            mW_FAC->set_sop(Elise_Set_Of_Palette::TheFullPalette());
            mW_FAC->set_title((aImg+"_ACORREL").c_str());
            ELISE_COPY(anIm->all_pts(), anIm->in(), mW_FAC->ogray());
        }
        if (mW_Final == 0)
        {
            mW_Final = Video_Win::PtrWStd(Pt2di(mSzW.x*mSzW.z, mSzW.y*mSzW.z), true, aZ*mSzW.z);
            mW_Final->set_sop(Elise_Set_Of_Palette::TheFullPalette());
            mW_Final->set_title((aImg+"_FINAL").c_str());
            ELISE_COPY(anIm->all_pts(), anIm->in(), mW_Final->ogray());
        }
    }
    mW_Final->clik_in();


    Pt2di aP;
    std::vector<cIntTieTriInterest> aListPI;
    cFastCriterCompute * mFastCC   = cFastCriterCompute::Circle(TT_DIST_FAST);

    cCutAutoCorrelDir< TIm2D<double,double> > mCutACD (mPic_TIm2D,Pt2di(0,0),TT_SZ_AUTO_COR /2.0 ,TT_SZ_AUTO_COR);
    for (aP.x=5 ; aP.x<aSzIm.x-5 ; aP.x++)
    {
        for (aP.y=5 ; aP.y<aSzIm.y-5 ; aP.y++)
        {
            int aCmp0 =  IsExtrema(mPic_TIm2D,aP);
            if (aCmp0)
            {
                eTypeTieTri aType = (aCmp0==1)  ? eTTTMax : eTTTMin;
                bool OKAutoCorrel = !mCutACD.AutoCorrel(aP,TT_SEUIL_CutAutoCorrel_INT,TT_SEUIL_CutAutoCorrel_REEL,TT_SEUIL_AutoCorrel);
                Pt2dr aFastQual =  FastQuality(mPic_TIm2D,aP,*mFastCC,aType==eTTTMax,Pt2dr(TT_PropFastStd,TT_PropFastConsec));
                bool OkFast = (aFastQual.x > TT_SeuilFastStd) && ( aFastQual.y> TT_SeuilFastCons);
                if (OkFast && OKAutoCorrel)
                    aListPI.push_back(cIntTieTriInterest(aP,aType,aFastQual.x + 2 * aFastQual.y));
                if (mW_Org)
                {
                    mW_Org->draw_circle_loc(Pt2dr(aP),1.5,ColOfType(mW_Org, aType));    // cercle grand => extrema
                    //mW_Org->draw_circle_loc(Pt2dr(aP),0.5,mW_Org->pdisc()(OkFast ? P8COL::yellow : P8COL::cyan)); //=> cercle petit => Fast : jaune  = valid ; cyan = non valid
                }
                if (mW_F)
                {
                    mW_F->draw_circle_loc(Pt2dr(aP),1.5,ColOfType(mW_F, aType));
                    if (!OkFast)
                        mW_F->draw_circle_loc(Pt2dr(aP),1.5,mW_F->pdisc()(P8COL::cyan));
                }
                if (mW_FAC)
                {
                    mW_FAC->draw_circle_loc(Pt2dr(aP),1.5,ColOfType(mW_FAC, aType));
                    if (!OKAutoCorrel)
                        mW_FAC->draw_circle_loc(Pt2dr(aP),1.5,mW_FAC->pdisc()(P8COL::yellow));
                }
                if (mW_Final && OKAutoCorrel && OkFast)
                {
                    mW_Final->draw_circle_loc(Pt2dr(aP),1.5,ColOfType(mW_Final, aType));
                }
            }
        }

    }
    cout<<"Nb Pts :"<<aListPI.size()<<endl;
    mW_FAC->clik_in();
    return EXIT_SUCCESS;
}

//-----------------TestGiang---------------//


//-----------------Test New format point Homologue---------------//
double cal_Residu( Pt3dr aPInter3D , vector<CamStenope*> & aVCamInter, vector<Pt2dr> & aVPtInter)
{
    double aMoy =0.0;
    for (uint aKCam=0; aKCam<aVCamInter.size(); aKCam++)
    {
        CamStenope * aCam = aVCamInter[aKCam];
        Pt2dr aPtMes = aVPtInter[aKCam];
        Pt2dr aPtRep = aCam->Ter2Capteur(aPInter3D);
        aMoy = aMoy + euclid(aPtMes, aPtRep);
    }
    return aMoy/aVCamInter.size();
}


Pt3di gen_coul(double val, double min, double max)
{
    if (val <= max && val >=min)
    {
        double Number = (val-min)/(max-min);
        int Green = round(255.0 - (255.0 * Number));
        int Red = round(255.0 * Number);
        int Blue = 0;
        return Pt3di(Red, Green, Blue);
    }
    else if (max == min && val == max)
        return Pt3di(0,255,0);  //green ?
    else
        return Pt3di(0,0,0);  //noir
}

Pt3di gen_coul_heat_map(double value , double minimum, double maximum)
{
    double ratio = 2 * (value-minimum) / (maximum - minimum);
    int b = int(ElMax(0.0, 255*(1 - ratio)));
    int r = int(ElMax(0.0, 255*(ratio - 1)));
    int g = 255 - b - r;
    return Pt3di(r,g,b);
}

Pt3di gen_coul_emp(int val)
{
    switch (val)
    {
        case 1:
            return Pt3di (255,0,0); //rouge
        case 2:
            return Pt3di (255,144,0);   //orange
        case 3:
            return Pt3di (255,255,0);   //jaune
        case 4:
            return Pt3di (140,255,0);   //vert jaunit
        case 5:
            return Pt3di (0,255,221);   //cyan
        default:
            return Pt3di (0,255,0);     //vert
    }

}


Pt3dr Intersect_Simple(const std::vector<CamStenope *> & aVCS,const std::vector<Pt2dr> & aNPts2D)
{

    std::vector<ElSeg3D> aVSeg;

    for (int aKR=0 ; aKR < int(aVCS.size()) ; aKR++)
    {
        ElSeg3D aSeg = aVCS.at(aKR)->F2toRayonR3(aNPts2D.at(aKR));
        //ElSeg3D aSeg = aVCS.at(aKR)->Capteur2RayTer(aNPts2D.at(aKR));
        aVSeg.push_back(aSeg);
    }

    Pt3dr aRes =  ElSeg3D::L2InterFaisceaux(0,aVSeg,0);
    return aRes;
}


void PlyPutForCC(string & aPlyResCC, vector<Pt3dr> & aVAllPtInter, vector<double> & aVResidu)
{
    ELISE_ASSERT(aVAllPtInter.size() > 0,"No Pts in PlyPutForCC");
    ELISE_ASSERT(aVResidu.size() == aVAllPtInter.size(),"Pts and Res dif size in PlyPutForCC");

    //int aNbS = aVAllPtInter.size();
    std::string aTypeXYZ = "float";

    bool aModeBin = 1; // mode bin
    std::string mode = aModeBin ? "wb" : "w";
    FILE * aFP = FopenNN(aPlyResCC,mode,"PlyPutForCC");
/*
    //Header
    fprintf(aFP,"ply\n");
    std::string aBinSpec =       MSBF_PROCESSOR() ?
                           "binary_big_endian":
                           "binary_little_endian" ;

    fprintf(aFP,"format %s 1.0\n",aModeBin?aBinSpec.c_str():"ascii");
    fprintf(aFP,"element vertex %d\n",aNbS);
    fprintf(aFP,"property %s x\n",aTypeXYZ.c_str());
    fprintf(aFP,"property %s y\n",aTypeXYZ.c_str());
    fprintf(aFP,"property %s z\n",aTypeXYZ.c_str());

    fprintf(aFP,"property float intensity\n");


    //fprintf(aFP,"property list uchar int vertex_indices\n");
    fprintf(aFP,"end_header\n");
*/
    for (uint i=0; i<aVAllPtInter.size(); i++)
    {
        Pt3dr aPt = aVAllPtInter[i];
        double aRes = aVResidu[i];
        fprintf(aFP,"%.3f %.3f %.3f %f\n",aPt.x,aPt.y,aPt.z,aRes);
    }

    ElFclose(aFP);

}


int TestGiangNewHomol_Main(int argc,char ** argv)
{
    //Test_Conv(argc, argv);


    string aDir = "./";
    string aSH="";
    string aOri="";
    Pt2dr aRange(0.0,0.0);
    bool relative = true;
    double resMaxTapas = 3.0;
    string aPlyRes="Res_";
    string aPlyEmp="Emp_";
    double seuilBH = 0.0;
    Pt3dr aHistoRes(0,0,0);
    ElInitArgMain
    (
          argc,argv,
          LArgMain()  << EAMC(aSH, "Homol New Format file",  eSAM_IsExistFile)
                      << EAMC(aOri, "Ori",  eSAM_IsExistDirOri),
          LArgMain()
                      << EAM(aDir,"Dir",true,"Directory , Def=./")
                      << EAM(resMaxTapas,"seuilRes",true,"threshold of reprojection error ")
                      << EAM(seuilBH,"seuilBH",true,"threshold for rapport B/H")
                      << EAM(aRange,"aRange",true,"range to colorize reprojection error ,green->red Def= colorize as relative (min->resMax)")
                      << EAM(aPlyRes,"PlyRes",true,"Ply's name output for residus - def=Cloud_Residu.ply")
                      << EAM(aPlyEmp,"PlyEmp",true,"Ply's name output for emplacement image - def=Cloud_Emp.ply")
                      << EAM(aHistoRes,"HistoRes",true,"Histogram of residue - [Min, Max, Nb Bin]")

    );

    if (EAMIsInit(&aRange))
    {
        relative = false;
    }

    cInterfChantierNameManipulateur*  aICNM = cInterfChantierNameManipulateur::BasicAlloc(aDir);
    StdCorrecNameOrient(aOri, aICNM->Dir());
    const std::string  aSHInStr = aSH;
    cSetTiePMul * aSHIn = new cSetTiePMul(0);
    aSHIn->AddFile(aSHInStr);

    cout<<"Total : "<<aSHIn->DicoIm().mName2Im.size()<<" imgs"<<endl;
    std::map<std::string,cCelImTPM *> VName2Im = aSHIn->DicoIm().mName2Im;
    // load cam for all Img
    // Iterate through all elements in std::map
    std::map<std::string,cCelImTPM *>::iterator it = VName2Im.begin();
    vector<CamStenope*> aVCam (VName2Im.size());
    while(it != VName2Im.end())
    {
        //std::cout<<it->first<<" :: "<<it->second->Id()<<std::endl;
        string aNameIm = it->first;
        int aIdIm = it->second->Id();
        CamStenope * aCam = aICNM->StdCamStenOfNames(aNameIm, aOri);
        aVCam[aIdIm] = aCam;
        it++;
    }
    cPlyCloud aCPlyRes;
    cPlyCloud aCPlyEmp;

    cout<<"VPMul - Nb Config: "<<aSHIn->VPMul().size()<<endl;
    std::vector<cSetPMul1ConfigTPM *> aVCnf = aSHIn->VPMul();

    vector<double> aVResidu;            // residue de tout les points dans pack
    vector<Pt3dr> aVAllPtInter;         // Coordonne 3D de tout les points dans pack
    vector<int> aVNbImgOvlap;           // Nb Overlape de tout les points 3D dans pack

    vector<int> aStats(aSHIn->NbIm());  // Vector contient multiplicite de pack, index d'element du vector <=> multiplicite, valeur d'element <=> nb point
    vector<int> aStatsInRange(aSHIn->NbIm()); // Vector contient multiplicite de pack dans 1 gamme de residue defini, index d'element du vector <=> multiplicite, valeur d'element <=> nb point
    vector<int> aStatsValid;            // Vector contient multiplicite existe de pack, valeur d'element <=> multiplicité

    int nbPtsInRange = 0;
    double resMax = 0.0;
    double resMin = DBL_MAX;
    for (uint aKCnf=1; aKCnf<aVCnf.size(); aKCnf++)
    {
        cSetPMul1ConfigTPM * aCnf = aVCnf[aKCnf];
        //cout<<"Cnf : "<<aKCnf<<" - Nb Imgs : "<<aCnf->NbIm()<<" - Nb Pts : "<<aCnf->NbPts()<<endl;
        std::vector<int> aVIdIm =  aCnf->VIdIm();

        for (uint aKPtCnf=0; aKPtCnf<uint(aCnf->NbPts()); aKPtCnf++)
        {
            vector<Pt2dr> aVPtInter;
            vector<CamStenope*> aVCamInter;


            for (uint aKImCnf=0; aKImCnf<aVIdIm.size(); aKImCnf++)
            {
                //cout<<aCnf->Pt(aKPtCnf, aKImCnf)<<" ";
                aVPtInter.push_back(aCnf->Pt(aKPtCnf, aKImCnf));
                aVCamInter.push_back(aVCam[aVIdIm[aKImCnf]]);
            }
            //cout<<endl;
            //Intersect aVPtInter:
            ELISE_ASSERT(aVPtInter.size() == aVCamInter.size(), "Size not coherent");
            ELISE_ASSERT(aVPtInter.size() > 1 && aVCamInter.size() > 1, "Nb faiseaux < 2");
            Pt3dr aPInter3D = Intersect_Simple(aVCamInter , aVPtInter);
            //calcul reprojection error :
            double resMoy = cal_Residu( aPInter3D , aVCamInter, aVPtInter);
            //cout<<resMoy<<endl;
            if (resMoy >= resMax)
            {
                if (resMoy <= resMaxTapas)
                {
                    resMax = resMoy;
                }
                else
                {
                    resMax =resMaxTapas;
                }
            }
            else
            {
                if (resMoy <= resMin)
                {
                    resMin = resMoy;
                }
            }
            if (resMoy <= resMaxTapas && resMoy>=0.0)
            {
                aVAllPtInter.push_back(aPInter3D);
                aVResidu.push_back(resMoy);
                aVNbImgOvlap.push_back(aVPtInter.size());
            }

        }
    }
    //ajout au nuage de point
    cout<<"Nb Pt 3d : "<<aVResidu.size();
    cout<<"Res max = "<<resMax<<" -res Min = "<<resMin<<endl;
    for (uint aKPt=0; aKPt<aVAllPtInter.size(); aKPt++)
    {
        //parcourir tout les points
        if (!relative)
        {
            aCPlyRes.AddPt(gen_coul_heat_map(aVResidu[aKPt], aRange.x,  aRange.y), aVAllPtInter[aKPt]);
        }
        else
        {
            aCPlyRes.AddPt(gen_coul_heat_map(aVResidu[aKPt], resMin,  resMax), aVAllPtInter[aKPt]);
        }
        aCPlyEmp.AddPt(gen_coul_emp(aVNbImgOvlap[aKPt]), aVAllPtInter[aKPt]);
        //===== stats Multiplicite ========
        int nbImgsVu1Pts = aVNbImgOvlap[aKPt];
        aStats[nbImgsVu1Pts]++;
        if (aVResidu[aKPt] >= aRange.x && aVResidu[aKPt]<=aRange.y)
        {
            aStatsInRange[nbImgsVu1Pts]++;
            nbPtsInRange++;
        }
        //voir si dans aStatsValid exist nbImgsVu1Pts
        if (!(std::find(aStatsValid.begin(), aStatsValid.end(), nbImgsVu1Pts) != aStatsValid.end()))
        {
            //si exist pas
            aStatsValid.push_back(nbImgsVu1Pts);
        }
        //==================================
    }
    aPlyRes = aPlyRes + aOri + ".ply";
    aPlyEmp = aPlyEmp + aOri + ".ply";
    string aPlyResCC = "Res_" + aOri +"_CC.txt";
    aCPlyRes.PutFile(aPlyRes);
    aCPlyEmp.PutFile(aPlyEmp);
    PlyPutForCC(aPlyResCC, aVAllPtInter, aVResidu);

    //===== stats Multiplicite ========
    ofstream statsFile;
    string aName = "Stats_" + aOri + ".txt";
    statsFile.open(aName.c_str());
    statsFile << "Stats Multiplicite"<<endl;
    statsFile << "Nb Pts Total : "<<aVAllPtInter.size()<<endl;
    statsFile << "NbMul  NbPts  %"<<endl;
    sort(aStatsValid.begin(), aStatsValid.end());
    for (uint ikLine=0; ikLine<aStatsValid.size(); ikLine++)
        statsFile << aStatsValid[ikLine] <<" "<<aStats[aStatsValid[ikLine]]<<" "<< (double(aStats[aStatsValid[ikLine]])/double(aVAllPtInter.size()))*100.0 <<endl;

    if (!relative)
    {
        statsFile << "============  Range  ==========="<<aRange.x<<" -> "<<aRange.y<<"  ============="<<endl;
        statsFile << "Stats Multiplicite"<<endl;
        statsFile << "Nb Pts Total : "<<nbPtsInRange<<endl;
        statsFile << "NbMul  NbPts  %"<<endl;
        for (uint ikLine=0; ikLine<aStatsValid.size(); ikLine++)
            statsFile << aStatsValid[ikLine] <<" "<<aStatsInRange[aStatsValid[ikLine]]<<" "<<(double(aStatsInRange[aStatsValid[ikLine]])/double(nbPtsInRange))*100.0 <<endl;
    }
    //==================================

    //===== Histo Residue ==============
    if (EAMIsInit(&aHistoRes))
    {
        double pas = (aHistoRes.y-aHistoRes.x)/aHistoRes.z;
        sort(aVResidu.begin(), aVResidu.end());
        int ind=0;
        statsFile<<endl<<endl<<"Histo Residue image : "<<endl<< "RangeMin RangeMax NbPts %"<<endl;
        for (double iPas=aHistoRes.x; iPas<aHistoRes.y; iPas=iPas+pas)
        {
            cout<<"Range : "<<iPas<<" - "<<iPas+pas<<" : ";
            statsFile<<iPas<<" "<<iPas+pas<<" ";
            double minRange=iPas; double maxRange=iPas+pas;
            int cntBin = 0;
            //For each range, extract vector element in range
            for (uint i=ind; i<aVResidu.size(); i++)
            {
                //parcour vector from index
                if (aVResidu[i] >= minRange)
                {
                    if (aVResidu[i] < maxRange)
                    {
                        cntBin++;
                    }
                    else
                    {
                        ind = i;
                        break;
                    }
                }
            }
            cout<<cntBin<<" "<<(double(cntBin)/aVAllPtInter.size())*100<<endl;
            statsFile<<cntBin<<" "<<(double(cntBin)/aVAllPtInter.size())*100<<endl;
        }
    }
    //==================================
    statsFile.close();


    cout<<"Nb Emplacement image : 1 rouge - 2 orange - 3 jaune - 4 vert jaune - 5 cyan - > 5 vert"<<endl;
    return EXIT_SUCCESS;
}

//= ===========================  TEST GIANG DISPLAY ALL HOMOL WITH 1 IMAGE ============================


int TestGiangDispHomol_Main(int argc,char ** argv)
{

    string aDir="./";
    string aSH="";
    string aPat;
    string aImg;
    double aZ=0.2;
    bool aTwoSens = false;
    ElInitArgMain
    (
          argc,argv,
          LArgMain()  << EAMC(aPat, "Pattern Image",  eSAM_IsPatFile)
                      << EAMC(aImg, "Image Master",  eSAM_IsExistFile),
          LArgMain()
                      << EAM(aSH,"SH",true,"Homol suffix")
                      << EAM(aZ,"Zoom",true,"0.2")
                      << EAM(aTwoSens,"TwoSens",true,"fault")

    );
    string mPatIm;
    SplitDirAndFile(aDir, mPatIm, aPat);
    cInterfChantierNameManipulateur *mICNM = cInterfChantierNameManipulateur::BasicAlloc(aDir);
    vector<string>mSetIm = *(mICNM->Get(mPatIm));
    ELISE_ASSERT(mSetIm.size()>0,"ERROR: No image found!");
    //============================================================
       //anExt = ExpTxt ? "txt" : "dat";
       string mNameHomol = "Homol"+aSH;


       string mKHIn =   std::string("NKS-Assoc-CplIm2Hom@")
                          +  std::string(mNameHomol)
                          +  std::string("@")
                          +  std::string("dat");


       vector<Pt2dr> mVPts;
       for (uint i=0; i<mSetIm.size(); i++)
       {
           string pic1 = aImg;
           string pic2 = mSetIm[i];
           cout<<pic1<<" "<<pic2<<endl;
           if (pic1 != pic2)
           {
               // ===== sens 1 =====
               string aHomoIn = mICNM->Assoc1To2(mKHIn, pic1, pic2, true);
               StdCorrecNameHomol_G(aHomoIn, aDir);
               cout<<"++ "<<aHomoIn;
               bool Exist= ELISE_fp::exist_file(aHomoIn);
               if (Exist)
               {
                   ElPackHomologue aPackIn;
                   bool Exist= ELISE_fp::exist_file(aHomoIn);
                   if (Exist)
                   {
                       aPackIn =  ElPackHomologue::FromFile(aHomoIn);
                       for (ElPackHomologue::const_iterator itP=aPackIn.begin(); itP!=aPackIn.end() ; itP++)
                       {

                           Pt2dr aP1 = itP->P1();  //Point img1
                           mVPts.push_back(aP1);

                       }
                   }
               }
               if (aTwoSens == true)
               {
                   // ===== sens 2 =====
                   string aHomoIn = mICNM->Assoc1To2(mKHIn, pic2, pic1, true);
                   StdCorrecNameHomol_G(aHomoIn, aDir);
                   cout<<"++ "<<aHomoIn;
                   bool Exist= ELISE_fp::exist_file(aHomoIn);
                   if (Exist)
                   {
                       ElPackHomologue aPackIn;
                       bool Exist= ELISE_fp::exist_file(aHomoIn);
                       if (Exist)
                       {
                           aPackIn =  ElPackHomologue::FromFile(aHomoIn);
                           for (ElPackHomologue::const_iterator itP=aPackIn.begin(); itP!=aPackIn.end() ; itP++)
                           {

                               Pt2dr aP2 = itP->P2();  //Point img1
                               mVPts.push_back(aP2);

                           }
                       }
                   }
               }
               //  ====== ====== ======
           }
       }

        cout<<endl<<endl<<"Total = "<<mVPts.size()<<" pts for img "<<aImg<<endl;


        Tiff_Im mPicTiff = Tiff_Im::UnivConvStd(aDir+aImg);
        Pt2di mImgSz = mPicTiff.sz();
        Video_Win * aWin;
        aWin = Video_Win::PtrWStd(Pt2di(mImgSz.x*aZ, mImgSz.y*aZ), true, Pt2dr(aZ,aZ));


        aWin->set_sop(Elise_Set_Of_Palette::TheFullPalette());
        aWin->set_title((aImg+"_ALLPTS").c_str());
        ELISE_COPY(mPicTiff.all_pts(), mPicTiff.in(), aWin->ogray());

        for (uint i=0; i<mVPts.size(); i++)
        {
            aWin->draw_circle_loc(mVPts[i], 2.0/aZ, aWin->pdisc()(P8COL::green));
        }


        aWin->clik_in();

    return EXIT_SUCCESS;
}

