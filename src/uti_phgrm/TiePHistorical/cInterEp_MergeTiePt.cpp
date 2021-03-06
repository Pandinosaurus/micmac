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

Ce logiciel est un programme informatique servant à la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  à l'utilisation,  à la modification et/ou au
développement et à la reproduction du logiciel par l'utilisateur étant
donné sa spécificité de logiciel libre, qui peut le rendre complexe à
manipuler et qui le réserve donc à des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités à charger  et  tester  l'adéquation  du
logiciel à leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement,
à l'utiliser et l'exploiter dans les mêmes conditions de sécurité.

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
aooter-MicMac-eLiSe-25/06/2007*/


void ReadXml(std::string & aImg1, std::string & aImg2, std::string aSubPatchXml, std::vector<std::string>& vPatchesL, std::vector<std::string>& vPatchesR, std::vector<cElHomographie>& vHomoL, std::vector<cElHomographie>& vHomoR)
{
    //cout<<aSubPatchXml<<endl;
    cSetOfPatches aSOMAF = StdGetFromSI(aSubPatchXml, SetOfPatches);

    std::list<cMes1Im>::const_iterator itIms = aSOMAF.Mes1Im().begin();

    cMes1Im aIms1 = * itIms;
    itIms++;
    cMes1Im aIms2 = * itIms;

    aImg1 = aIms1.NameIm();
    aImg2 = aIms2.NameIm();

    for(std::list<cOnePatch1I>::const_iterator itF = aIms1.OnePatch1I().begin() ; itF != aIms1.OnePatch1I().end() ; itF++)
    {
        cOnePatch1I aMAF = *itF;
        vPatchesL.push_back(aMAF.NamePatch());
        vHomoL.push_back(aMAF.PatchH());
    }

    for(std::list<cOnePatch1I>::const_iterator itF = aIms2.OnePatch1I().begin() ; itF != aIms2.OnePatch1I().end() ; itF++)
    {
        cOnePatch1I aMAF = *itF;
        vPatchesR.push_back(aMAF.NamePatch());
        vHomoR.push_back(aMAF.PatchH());
    }
}

void MergeTiePt(std::string input_dir, std::string output_dir, std::string inSH, std::string outSH, std::string aSubPatchXml)
{
    std::string aImg1, aImg2;
    std::vector<std::string> vPatchesL, vPatchesR;
    std::vector<cElHomographie> vHomoL, vHomoR;

    ReadXml(aImg1, aImg2, input_dir+"/"+aSubPatchXml, vPatchesL, vPatchesR, vHomoL, vHomoR);

    //cout<<vPatchesL.size()<<", "<<vPatchesR.size()<<", "<<vHomoL.size()<<", "<<vHomoR.size()<<"\n";

    std::string aCom = "mm3d SEL" + BLANK + output_dir + BLANK + aImg1 + BLANK + aImg2 + BLANK + "KH=NT SzW=[600,600] SH="+outSH;
    cout<<aCom<<endl;

    std::string aSHDir = output_dir + "/Homol" + outSH+"/";
    ELISE_fp::MkDir(aSHDir);
    std::string aNewDir = aSHDir + "Pastis" + aImg1;
    ELISE_fp::MkDir(aNewDir);
    std::string aNameFile = aNewDir + "/"+aImg2+".txt";
    FILE * fpTiePt1 = fopen(aNameFile.c_str(), "w");

    aNewDir = aSHDir + "Pastis" + aImg2;
    ELISE_fp::MkDir(aNewDir);
    aNameFile = aNewDir + "/"+aImg1+".txt";
    FILE * fpTiePt2 = fopen(aNameFile.c_str(), "w");

    ElPackHomologue aPackMerged;

    std::string aDir_inSH = input_dir + "/Homol" + inSH+"/";
    /*
    ifstream in(input_dir+input_pairs);
    std::string s;
    while(getline(in,s))
    {
    */
    for(unsigned int i=0; i<vPatchesL.size(); i++)
    {
        std::string aPatch1 = vPatchesL[i];
        cElHomographie  aFstH = vHomoL[i];

        for(unsigned int j=0; j<vPatchesR.size(); j++)
        {
            //printf("%s\n", s.c_str());
            std::string aPatch2 = vPatchesR[j];
            cElHomographie  aSndH = vHomoR[j];

            std::string aNameIn = aDir_inSH +"Pastis" + aPatch1 + "/"+aPatch2+".txt";

            bool bReverse = false;
            if (ELISE_fp::exist_file(aNameIn) == false)
            {
                bReverse = true;
                aNameIn = aDir_inSH +"Pastis" + aPatch2 + "/"+aPatch1+".txt";
                if (ELISE_fp::exist_file(aNameIn) == false)
                {
                    //cout<<aNameIn<<"didn't exist hence skipped."<<endl;
                    continue;
                }
            }

            int nTiePtNum = 0;
            ElPackHomologue aPackInLoc =  ElPackHomologue::FromFile(aNameIn);
            for (ElPackHomologue::iterator itCpl=aPackInLoc.begin();itCpl!=aPackInLoc.end() ; itCpl++)
            {
                /*
                ElCplePtsHomologues tiept = itCpl->ToCple();
                printf("ori:  %lf %lf %lf %lf\n", tiept.P1().x, tiept.P1().y, tiept.P2().x, tiept.P2().y);
*/
                Pt2dr aP1, aP2;
                if(bReverse == false)
                {
                    aP1 = itCpl->ToCple().P1();
                    aP2 = itCpl->ToCple().P2();
                }
                else
                {
                    aP2 = itCpl->ToCple().P1();
                    aP1 = itCpl->ToCple().P2();
                }
                aP1 = aFstH(aP1);
                aP2 = aSndH(aP2);
                //printf("merged:  %lf %lf %lf %lf\n", aP1.x, aP1.y, aP2.x, aP2.y);

                aPackMerged.Cple_Add(ElCplePtsHomologues(aP1,aP2,1.0));
                nTiePtNum++;
            }
            /*
            cout<<aNameIn<<endl;
            cout<<"nTiePtNum: "<<nTiePtNum<<";  ";
            cout<<"HX: "<<aSndH.HX().CoeffX()<<" "<<aSndH.HX().CoeffY()<<" "<<aSndH.HX().Coeff1()<<";  ";
            cout<<"HY: "<<aSndH.HY().CoeffX()<<" "<<aSndH.HY().CoeffY()<<" "<<aSndH.HY().Coeff1()<<";  ";
            cout<<"HZ: "<<aSndH.HZ().CoeffX()<<" "<<aSndH.HZ().CoeffY()<<" "<<aSndH.HZ().Coeff1()<<endl;
            */
        }
    }

    int nTotalTiePtNum = 0;
    for (ElPackHomologue::iterator itCpl=aPackMerged.begin();itCpl!=aPackMerged.end() ; itCpl++)
    {
        ElCplePtsHomologues tiept = itCpl->ToCple();
        fprintf(fpTiePt1, "%lf %lf %lf %lf\n", tiept.P1().x, tiept.P1().y, tiept.P2().x, tiept.P2().y);
        fprintf(fpTiePt2, "%lf %lf %lf %lf\n", tiept.P2().x, tiept.P2().y, tiept.P1().x, tiept.P1().y);
        nTotalTiePtNum++;
    }
    fclose(fpTiePt1);
    fclose(fpTiePt2);

    cout<<"Tie point number after merged: "<<nTotalTiePtNum<<endl;
}

int MergeTiePt_main(int argc,char ** argv)
{
   cCommonAppliTiepHistorical aCAS3D;

   std::string aDir;

   std::string aOutDir = "";

   ElInitArgMain
    (
        argc,argv,
        LArgMain()  << EAMC(aDir,"Work directory"),
        LArgMain()
                    //<< aCAS3D.ArgBasic()
               << EAM(aOutDir, "OutDir", true, "Output directory of the merged tie points, Def=Work directory")
                    << aCAS3D.ArgMergeTiePt()
    );

   if(aOutDir.length() == 0)
       aOutDir = aDir;

   if(aCAS3D.mMergeTiePtOutSH.length() == 0)
       aCAS3D.mMergeTiePtOutSH = "-" + StdPrefix(aCAS3D.mHomoXml);

   //cout<<aDir<<",,,"<<aCAS3D.mHomoXml<<endl;

   MergeTiePt(aDir, aOutDir, aCAS3D.mMergeTiePtInSH, aCAS3D.mMergeTiePtOutSH, aCAS3D.mHomoXml);

   return EXIT_SUCCESS;
}
