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

#include "TiePHistorical.h"



/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant à la aise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, aodifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de aodification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les aêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  à l'utilisation,  à la aodification et/ou au
développement et à la reproduction du logiciel par l'utilisateur étant
donné sa spécificité de logiciel libre, qui peut le rendre complexe à
manipuler et qui le réserve donc à des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités à charger  et  tester  l'adéquation  du
logiciel à leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement,
à l'utiliser et l'exploiter dans les aêmes conditions de sécurité.

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
aooter-MicMac-eLiSe-25/06/2007*/

void DSM_Equalization(std::string aDSMDir, std::string aDSMFile, std::string aOutImg, double dSTDRange)
{
    aDSMDir += '/';
    cout<<aDSMDir + aDSMFile<<endl;
    cXML_ParamNuage3DMaille aNuageIn = StdGetObjFromFile<cXML_ParamNuage3DMaille>
    (
    aDSMDir + aDSMFile,
    StdGetFileXMLSpec("SuperposImage.xml"),
    "XML_ParamNuage3DMaille",
    "XML_ParamNuage3DMaille"
    );

    Pt2di aDSMSz = aNuageIn.NbPixel();

    cImage_Profondeur aImDSM = aNuageIn.Image_Profondeur().Val();

    std::string aImName = aDSMDir + aImDSM.Image();
    Tiff_Im aImDSMTif(aImName.c_str());
    TIm2D<float,double> aTImDSM(aDSMSz);
    ELISE_COPY
    (
    aTImDSM.all_pts(),
    aImDSMTif.in(),
    aTImDSM.out()
    );

    std::string aMasqName = aDSMDir + aImDSM.Masq();
    Tiff_Im aImMasqTif(aMasqName.c_str());
    TIm2D<float,double> aTImMasq(aDSMSz);
    ELISE_COPY
    (
    aTImMasq.all_pts(),
    aImMasqTif.in(),
    aTImMasq.out()
    );

    //cout<<aTImMasq.get(Pt2di(0, 0))<<",,,,,,,"<<aTImMasq.get(Pt2di(1000, 800))<<"\n";

    int nValidPxNum = 0;

    int i, j;
    double dMean = 0;
    for(i=0; i<aDSMSz.x; i++)
    {
        for(j=0; j<aDSMSz.y; j++)
        {
            int nVal =  aTImMasq.get(Pt2di(i, j));
            if(nVal > 0)
            {
                double dZ =  aTImDSM.get(Pt2di(i, j));
                dMean += dZ;
                nValidPxNum++;
            }
        }
    }
    dMean /= nValidPxNum;

    double dSTD = 0;
    for(i=0; i<aDSMSz.x; i++)
    {
        for(j=0; j<aDSMSz.y; j++)
        {
            int nVal =  aTImMasq.get(Pt2di(i, j));
            if(nVal > 0)
            {
                double dZ =  aTImDSM.get(Pt2di(i, j));
                dSTD += pow(dZ-dMean, 2);
            }
        }
    }
    dSTD = pow(dSTD/nValidPxNum, 0.5);

    cout<<"dMean: "<<dMean<<"; dSTD: "<<dSTD<<endl;

    double aMaxAlti = dMean+dSTD*dSTDRange;
    double aMinAlti = dMean-dSTD*dSTDRange;

    double dScale = 255/(aMaxAlti-aMinAlti);
    double dTranslation = aMinAlti;

    if(aOutImg.length()==0)
        aOutImg = StdPrefix(aImName) + "_gray.tif";
    else
        aOutImg = aDSMDir + aOutImg;
    //cout<<aOutImg<<endl;

    //cout<<dScale<<",,,"<<dTranslation<<endl;
    //cout<<aOutImg<<endl;

    std::string aSubStr = " - " + std::to_string(dTranslation);
    if(dTranslation < 0)
        aSubStr = " + " + std::to_string(fabs(dTranslation));
    std::string aComNikrup = MMBinFile(MM3DStr) + "Nikrup \"* "  + std::to_string(dScale) + aSubStr + " " + aImName + "\" " + aOutImg;
    std::string aComTo8Bits = MMBinFile(MM3DStr) + "to8Bits " + aOutImg + " UseSigne=false";

    std::string aComRename = "mv " + StdPrefix(aOutImg) + "_8Bits." + StdPostfix(aOutImg) + " " + aOutImg;

    cout<<aComNikrup<<"\n"<<aComTo8Bits<<"\n"<<aComRename<<endl;
    System(aComNikrup);
    System(aComTo8Bits);
    System(aComRename);

    cout<<"xdg-open "<<aOutImg<<endl;
}
/*
void TransformDSM(std::string aFileOut)
{
    cXml_Map2D aMap;
    std::list<cXml_Map2DElem> aLMEL;
    cXml_Map2DElem aMapEL;
    cXmlHomogr Hom;

    //double aSc3= aSc1 * aSc2;
    Hom.X() =2.0;
       Hom.Tr() = Pt2dr(500,500);

       aMapEL.Homot() = Hom;

      aLMEL.push_back(aMapEL);
      aMap.Maps() = aLMEL;

      MakeFileXML(aMap, aFileOut);
}
*/
int DSM_Equalization_main(int argc,char ** argv)
{
   cCommonAppliTiepHistorical aCAS3D;

   std::string aDSMDir;
   std::string aDSMFile = "MMLastNuage.xml";
   std::string aOutImg;

   ElInitArgMain
    (
        argc,argv,
        LArgMain()  << EAMC(aDSMDir, "DSM directory"),
        LArgMain()
                    //<< aCAS3D.ArgBasic()
                    << aCAS3D.ArgDSM_Equalization()
                    << EAM(aDSMFile, "DSMFile", true, "DSM File, Def=MMLastNuage.xml")
                    << EAM(aOutImg, "OutImg", true, "Output image name")

    );

   //TransformDSM("/home/lulin/Documents/zll/TestLulinCodeInMicMac/pipeline/map.xml");

   DSM_Equalization(aDSMDir, aDSMFile, aOutImg, aCAS3D.mSTDRange);

   return EXIT_SUCCESS;
}
