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

#include "NewOri.h"

#define HOM_NbMinPTs 20

class cOrHom_AttrSom;
class cOrHom_AttrASym;
class cOrHom_AttrArc;
class cAppli_Hom1Im;

typedef  ElSom<cOrHom_AttrSom,cOrHom_AttrArc>         tSomGT;
typedef  ElArc<cOrHom_AttrSom,cOrHom_AttrArc>         tArcGT;
typedef  ElSomIterator<cOrHom_AttrSom,cOrHom_AttrArc> tItSGT;
typedef  ElArcIterator<cOrHom_AttrSom,cOrHom_AttrArc> tItAGT;
typedef  ElGraphe<cOrHom_AttrSom,cOrHom_AttrArc>      tGrGT;
typedef  ElSubGraphe<cOrHom_AttrSom,cOrHom_AttrArc>   tSubGrGT;


/************************************************/
/*                                              */
/*         cOrHom_AttrSom                       */
/*                                              */
/************************************************/

class cOrHom_AttrSom
{
     public :
        cOrHom_AttrSom(const std::string & aName,cAppli_Hom1Im &);
        cOrHom_AttrSom();
        const std::string & Name() {return mName;}
        cAppli_Hom1Im & Appli() {return *mAppli;}
     private :
        std::string mName;
        cAppli_Hom1Im * mAppli;
};

cOrHom_AttrSom::cOrHom_AttrSom(const std::string & aName,cAppli_Hom1Im & anAppli) :
   mName  (aName),
   mAppli (&anAppli)
{
}

cOrHom_AttrSom::cOrHom_AttrSom() :
   mAppli (0)
{
}


/************************************************/
/*                                              */
/*         cOrHom_AttrArc                       */
/*                                              */
/************************************************/

          // cElHomographie(const cXmlHomogr &);
class cOrHom_AttrArcSym
{
     public :
        cOrHom_AttrArcSym(const cXml_Ori2Im &);
        const cXml_Ori2Im & Xml() const {return mXmlO;}
        cElHomographie Hom(bool Dir);

     private :
           cXml_Ori2Im  mXmlO;
};

cOrHom_AttrArcSym::cOrHom_AttrArcSym(const cXml_Ori2Im & aXml) :
   mXmlO (aXml)
{
}

cElHomographie cOrHom_AttrArcSym::Hom(bool aDir)
{
    cElHomographie aRes(mXmlO.Geom().Val().HomWithR().Hom());
    if (! aDir)
       aRes = aRes.Inverse();

    return aRes;
}
         // =================================


class cOrHom_AttrArc
{
    public :
        cOrHom_AttrArc(cOrHom_AttrArcSym * ,bool Direct);
        void GetDistribGaus(std::vector<Pt2dr> & aVPts,int aN);
    private :
         cOrHom_AttrArcSym * mASym;
         bool                mDirect;
         cElHomographie      mHom;
};

cOrHom_AttrArc::cOrHom_AttrArc(cOrHom_AttrArcSym * anAsym ,bool Direct) :
   mASym(anAsym),
   mDirect (Direct),
   mHom    (anAsym->Hom(Direct))
{
}

void cOrHom_AttrArc::GetDistribGaus(std::vector<Pt2dr> & aVPts,int aN)
{
}

/*
*/

/************************************************/
/*                                              */
/*         cAppli_Hom1Im                        */
/*                                              */
/************************************************/


class cAppli_Hom1Im : public cCommonMartiniAppli
{
    public :
        cAppli_Hom1Im(int argc,char ** argv,bool ModePrelim);
        tGrGT & Gr() {return mGr;}

    private:
        tSomGT * AddAnIm(const std::string & aName,bool CanCreate);
        tSomGT * GetAnIm(const std::string & aName);
        void   AddArc(tSomGT * aS1,tSomGT * aS2);

        bool        mModePrelim;
        std::string mPat;
        std::string mNameC;
        cElemAppliSetFile mEASF;
        std::map<std::string,tSomGT *> mMapS;
        std::vector<tSomGT *>          mVecS;
        tSomGT *                       mSomC;
        tGrGT                          mGr;
        cNewO_NameManager *            mNM;

};

void   cAppli_Hom1Im::AddArc(tSomGT * aS1,tSomGT * aS2)
{
/*
     if (mShow) 
        std::cout << "  ARC " <<  aS1->attr().Name() << " " <<  aS2->attr().Name() << "\n";
*/

     cXml_Ori2Im  aXml = mNM->GetOri2Im(aS1->attr().Name(),aS2->attr().Name());

     if ((!aXml.Geom().IsInit())  || (aXml.NbPts() < HOM_NbMinPTs))
        return;


    cOrHom_AttrArcSym * anAttr = new cOrHom_AttrArcSym(aXml);

    aS1->attr().Appli().Gr().add_arc(*aS1,*aS2,cOrHom_AttrArc(anAttr,true),cOrHom_AttrArc(anAttr,false));
    // add_arc

}


tSomGT * cAppli_Hom1Im::AddAnIm(const std::string & aName,bool CanCreate)
{
   if (mMapS[aName] == 0)
   {
      if (!CanCreate)
      {
         // std::cout << "For name=" << aName << "\n";
         // ELISE_ASSERT(false,"cAppli_Hom1Im::AddAnIm cannot get Image");
         return 0;
      }
      mMapS[aName]  = &(mGr.new_som(cOrHom_AttrSom(aName,*this)));
      mVecS.push_back(mMapS[aName]);

/*
     if (mShow) 
         std::cout <<" Add " << aName << "\n";
*/
   }

   return mMapS[aName];
}
tSomGT * cAppli_Hom1Im::GetAnIm(const std::string & aName) {return AddAnIm(aName,false);}


cAppli_Hom1Im::cAppli_Hom1Im(int argc,char ** argv,bool aModePrelim) :
   mModePrelim (aModePrelim),
   mSomC       (0),
   mNM         (0)
{
   ElInitArgMain
   (
        argc,argv,
        LArgMain() << EAMC(mPat,"Central image"),
        LArgMain() << ArgCMA()
   );

   mEASF.Init(mPat);
   mNM =   NM(mEASF.mDir);
   const cInterfChantierNameManipulateur::tSet * aVN = mEASF.SetIm();

   for(int aK=0 ; aK<int(aVN->size()) ; aK++)
   {
       AddAnIm((*aVN)[aK],true);
   }

   // Cas preliminaire, on rajoute tous les sommets connexe
   if (mModePrelim)
   {
       ELISE_ASSERT(aVN->size()==1,"Expect just one image in preliminary mode");
       mNameC = (*aVN)[0];
       mSomC =  AddAnIm(mNameC,false);
       std::list<std::string> aLC = mNM->Liste2SensImOrientedWith(mNameC);
       for (std::list<std::string>::const_iterator itL=aLC.begin() ; itL!=aLC.end() ; itL++)
            AddAnIm(*itL,true);
   }
    

   // Ajout des arcs

   for (int aKS=0 ; aKS<int(mVecS.size()) ; aKS++)
   {
       tSomGT * aS1 = mVecS[aKS];
       std::list<std::string> aLC = mNM->ListeImOrientedWith(aS1->attr().Name());
       for (std::list<std::string>::const_iterator itL=aLC.begin() ; itL!=aLC.end() ; itL++)
       {
           tSomGT  * aS2 =  GetAnIm(*itL);
           if (aS2)
           {
              AddArc(aS1,aS2);
           }
       }
   }
}


int  TestNewOriHom1Im_main(int argc,char ** argv)
{
    cAppli_Hom1Im anAppli(argc,argv,true);

     return EXIT_SUCCESS;
}



/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant a  la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est regi par la licence CeCILL-B soumise au droit fran�ais et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusee par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilite au code source et des droits de copie,
de modification et de redistribution accordes par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitee.  Pour les m�mes raisons,
seule une responsabilite restreinte pese sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concedants successifs.

A cet egard  l'attention de l'utilisateur est attiree sur les risques
associes au chargement,    l'utilisation,    la modification et/ou au
developpement et  la reproduction du logiciel par l'utilisateur etant
donne sa specificite de logiciel libre, qui peut le rendre complexe 
manipuler et qui le reserve donc   des developpeurs et des professionnels
avertis possedant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invites a  charger  et  tester  l'adequation  du
logiciel a  leurs besoins dans des conditions permettant d'assurer la
securite de leurs systemes et ou de leurs donnees et, plus generalement,
a  l'utiliser et l'exploiter dans les m�mes conditions de securite.

Le fait que vous puissiez acceder a  cet en-t�te signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accepte les
termes.
Footer-MicMac-eLiSe-25/06/2007*/


