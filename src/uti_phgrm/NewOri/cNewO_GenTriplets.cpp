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


/*
    Cout de recouvrement pour les triplets :

    Pour un arc donne S1S2, la distribution P1 est calculée de manière linéaire

       P1, x y  => K  = (x -x0) / (x1-x0)  + (x -x0) / (x1-x0) * Nx

   Pour chaque Sommet S3 on calcule aussi la distribution des point multiple sur I1 : Nb3(K)


   Une fonction de ponderation des case est calculee :

     Pds(K) = srqt(Nb1(aK))  et un Poids entier PdsI(K) = E(100 * Pds(K)/Max(Pds(aK)))

  Une fonction de densitee est calculée :

     D(aK) = Nb3(aK) /  Nb(aK)
 
  Pour attenue cette fonction, avecun coeff A (A=10, signifie que a partir de 1/10 on diminue l'influence) :

     D'(aK) = A * D(K) /( A D +1)

  Et la Valeur entiere :

     DI(aK) = E(100 *  D'(aK) * (A+1)/A)


  Le Gain qu'apporte un triplet sur une case , soit D12 la distance entre  les deux sommets

   Soit DLim la valeur d'attenation du B/H  D'12 = D12 * DLim (D12+DLim)
 
   Gain = Dens1 * ( D12 * Dens2 +  DLim * (1-D12))



*/

#include "NewOri.h"

class cGTrip_AttrSom;
class cGTrip_AttrASym;
class cGTrip_AttrArc;
class cTripletInt;
class cAppli_GenTriplet;

typedef  ElSom<cGTrip_AttrSom,cGTrip_AttrArc>         tSomGT;
typedef  ElArc<cGTrip_AttrSom,cGTrip_AttrArc>         tArcGT;
typedef  ElSomIterator<cGTrip_AttrSom,cGTrip_AttrArc> tItSGT;
typedef  ElArcIterator<cGTrip_AttrSom,cGTrip_AttrArc> tItAGT;
typedef  ElGraphe<cGTrip_AttrSom,cGTrip_AttrArc>      tGrGT;
typedef  ElSubGraphe<cGTrip_AttrSom,cGTrip_AttrArc>   tSubGrGT;


typedef  cFixedMergeTieP<3,Pt2df>    tElM;
typedef  cFixedMergeStruct<3,Pt2df>  tMapM;
typedef  std::list<tElM *>           tListM;

class cGTrip_AttrSom
{
     public :
         cGTrip_AttrSom(int aNum,const std::string & aNameIm,cAppli_GenTriplet & anAppli) ;

         cGTrip_AttrSom() {}

         const int & Num() const {return mNum;}
         const std::string & Name() const {return mName;}
         cNewO_OneIm & Im() {return *mIm;}

         void InitTriplet(tSomGT*,tArcGT *);
         void FreeTriplet();


     private :
         void TripletAddArc(tArcGT *,int aNum1,int aNum2);
         int            mNum;
         std::string    mName;
         cNewO_OneIm *  mIm;
         Pt3dr          mC3;

         tMapM * mMerged;
};

class cGTrip_AttrASym
{
     public :
        cGTrip_AttrASym(const cXml_Ori2Im & aXml) :
              mXML (aXml)
        {
        }
        std::vector<Pt2df> & VP1() {return mVP1;}
        std::vector<Pt2df> & VP2() {return mVP2;}
        Pt2df & InfP1() {return mInfP1;}
        Pt2df & SupP1() {return mSupP1;}
     private  :
        cGTrip_AttrASym(const cGTrip_AttrASym & aXml) ; // N.I.
        cXml_Ori2Im mXML;
        std::vector<Pt2df> mVP1;
        std::vector<Pt2df> mVP2;
        Pt2df              mInfP1;
        Pt2df              mSupP1;
};

class cGTrip_AttrArc
{
    public :
        cGTrip_AttrArc(ElRotation3D aR,cGTrip_AttrASym * anASym,bool ASymIsDir) :
             mRot   (aR),
             mASym  (anASym),
             mASDir (ASymIsDir)
        {
        }
        cGTrip_AttrASym & ASym() {return *mASym;}
        bool IsDirASym() const {return mASDir;}
        std::vector<Pt2df> & VP1() {return mASDir ? mASym->VP1() : mASym->VP2();}
        std::vector<Pt2df> & VP2() {return mASDir ? mASym->VP2() : mASym->VP1();}
        const ElRotation3D & Rot() const {return mRot;}

    private  :
        ElRotation3D      mRot;
        cGTrip_AttrASym * mASym;
        bool              mASDir;
};

class cTripletInt
{
    public :
       cTripletInt(int aK1,int aK2, int aK3) :
              mK1 (ElMin3(aK1,aK2,aK3)),
              mK3 (ElMax3(aK1,aK2,aK3)),
              mK2 (aK1+aK2+aK3-mK1-mK3)
        {
        }

        bool operator < (const cTripletInt & aT2) const
        {
             if (mK1 < aT2.mK1) return true;
             if (mK1 > aT2.mK1) return false;
             if (mK2 < aT2.mK2) return true;
             if (mK2 > aT2.mK2) return false;
             return mK3 < aT2.mK3;
        }

        int mK1;
        int mK3;
        int mK2;
};


class cAppli_GenTriplet
{
    public :
       cAppli_GenTriplet(int argc,char ** argv);
       void  GenTriplet();
       cNewO_NameManager & NM() {return *mNM;}

       int ToIndex(const Pt2df &  aP) const;
    private :
       bool  AddTriplet(tSomGT & aS1,tSomGT & aS2,tSomGT & aS3);
       void  GenTriplet(tArcGT & anArc);
       void AddSomTmp(tSomGT & aS);

       tGrGT                mGrT;
       tSubGrGT             mSubAll;
       std::string          mFullName;
       std::string          mNameOriCalib;
       cElemAppliSetFile    mEASF;
       cNewO_NameManager *  mNM;
       std::map<std::string,tSomGT *> mMapS;
       std::vector<tSomGT *>          mVecS;

       std::set<cTripletInt>          mTriplets;

       // Voisin de l'arc, hors de l'arc lui meme
       std::vector<tSomGT *>         mVSomVois;
       int                           mFlagVois;
       tArcGT *                      mCurArc;
       bool                          mShow;
       Pt2df                         mPInf;
       Pt2df                         mPSup;
       double                        mStep;
       Pt2di                         mSz;
};

/*********************************************************/
/*                                                       */
/*               cGTrip_AttrSom                          */
/*                                                       */
/*********************************************************/

cGTrip_AttrSom::cGTrip_AttrSom(int aNum,const std::string & aNameIm,cAppli_GenTriplet & anAppli) :
     mNum (aNum),
     mName (aNameIm),
     mIm   (new cNewO_OneIm(anAppli.NM(),mName))
{
}

void cGTrip_AttrSom::TripletAddArc(tArcGT * anArc,int aNum1,int aNum2)
{
     ELISE_ASSERT(anArc!=0,"cGTrip_AttrSom::TripletAddArc, arc=0");
     std::vector<Pt2df> &  aVP1 =  anArc->attr().VP1();
     std::vector<Pt2df> &  aVP2 =  anArc->attr().VP2();

     for (int aK=0 ; aK<int(aVP1.size()) ; aK++)
         mMerged->AddArc(aVP1[aK],aNum1,aVP2[aK],aNum2);
}




void cGTrip_AttrSom::InitTriplet(tSomGT * aSom,tArcGT * anA12)
{
      tGrGT & aGr = aSom->gr();
      tArcGT* anA13 = aGr.arc_s1s2(anA12->s1(),*aSom);
      tArcGT* anA23 = aGr.arc_s1s2(anA12->s2(),*aSom);

      static std::vector<Pt2df> aVP1;
      static std::vector<Pt2df> aVP2;
      static std::vector<Pt2df> aVP3;
      aVP1.clear();
      aVP2.clear();
      aVP3.clear();


      mMerged = new  tMapM;
      TripletAddArc(anA12,0,1);
      TripletAddArc(anA13,0,2);
      TripletAddArc(anA23,1,2);
      mMerged->DoExport();

      const tListM aLM =  mMerged->ListMerged();
      int aNb33=0;
      for (tListM::const_iterator itM=aLM.begin() ; itM!=aLM.end() ; itM++)
      {
          if ((*itM)->NbSom()==3 )
          {
              aVP1.push_back((*itM)->GetVal(0));
              aVP2.push_back((*itM)->GetVal(1));
              aVP3.push_back((*itM)->GetVal(2));
          }
          if ((*itM)->NbArc()==3 ) aNb33++;
      }
      // std::list<cFixedMergeTieP<3,Pt2df> >
      if (int(aVP1.size()) <TNbMinPMul) 
      {
         FreeTriplet();
         return;
      }
      // std::cout << "InitTriplet " << aNb << " " << aNb3 << " " << aNb33 << "\n";

      // Pour qu'il y ait intersection 
      // Det(L C1C3 + C12,  R3U3, ,R2U) = 0
      std::vector<double> aVL13; 
      std::vector<double> aVL23; 
      std::vector<double> aVLInv; 
      ElRotation3D aR21 = anA12->attr().Rot();
      ElRotation3D aR31 = anA13->attr().Rot();
      ElRotation3D aR32 = anA23->attr().Rot();
      ElRotation3D aR31Bis =  aR21 * aR32;


      Pt3dr aC1(0,0,0);
      Pt3dr aC2 = aR21.tr();
      Pt3dr aC3 = aR31.tr();
      Pt3dr aV3Bis = aR31Bis.tr() - aC2;


      int aStep = ElMin(10,ElMax(1,int(aVP1.size())/50));

      for (int aKL=0 ; aKL<int(aVP1.size()) ; aKL+=aStep)
      {
          Pt3dr aU31(aR31.ImVect(Pt3dr(aVP3[aKL].x,aVP3[aKL].y,1.0)));
          Pt3dr aU2(aR21.ImVect(Pt3dr(aVP2[aKL].x,aVP2[aKL].y,1.0)));

          Pt3dr aU1(Pt3dr(aVP1[aKL].x,aVP1[aKL].y,1.0));
          Pt3dr aU31Bis(aR31Bis.ImVect(Pt3dr(aVP3[aKL].x,aVP3[aKL].y,1.0)));


          double aDet = Det(aC2,aU31,aU2);
          if (aDet)
              aVL13.push_back(Det(aC3,aU31,aU2) / aDet);

          aDet =  Det(aV3Bis,aU31Bis,aU1);
          if (aDet)
              aVL23.push_back( -Det(aC2,aU31Bis,aU1) /aDet);
      }
      Pt3dr aC31 = aC3 / MedianeSup(aVL13);
      Pt3dr aC32 = aC2 + aV3Bis * MedianeSup(aVL23);
      mC3 = (aC31 + aC32) / 2.0;


      std::cout << "DDDDd " << euclid(aC31-aC32) << "\n";

      FreeTriplet();
}


void cGTrip_AttrSom:: FreeTriplet()
{
   mMerged->Delete();
   delete mMerged;
}



/*
         void InitTriplet();
         void EndTriplet();
*/
/*********************************************************/
/*                                                       */
/*              cAppli_GenTriplet                        */
/*                                                       */
/*********************************************************/

void cAppli_GenTriplet::AddSomTmp(tSomGT & aS)
{
   if ( aS.flag_kth(mFlagVois))
      return;

   aS.flag_set_kth_true(mFlagVois);
   mVSomVois.push_back(&aS);


   aS.attr().InitTriplet(&aS,mCurArc);
}


void cAppli_GenTriplet::GenTriplet(tArcGT & anArc)
{
    if (!anArc.attr().IsDirASym() ) return;
    mCurArc = & anArc; 

    mPInf = anArc.attr().ASym().InfP1();
    mPSup = anArc.attr().ASym().SupP1();
    Pt2df aPLarg = mPSup-mPInf;
    mStep = ElMax(aPLarg.x,aPLarg.y) / TNbCaseP1;
    mSz = Pt2di(round_up(aPLarg.x/mStep),round_up(aPLarg.y/mStep));


    for(tItAGT itA=anArc.s1().begin(mSubAll) ; itA.go_on() ; itA++)
    {
       tSomGT & aS3 = (*itA).s2();
       if (mGrT.arc_s1s2(anArc.s2(),aS3))
       {
          AddSomTmp(aS3);
       }
    }


    // Vider les structure temporaires
    for (int aKS=0 ; aKS<int(mVSomVois.size()) ; aKS++)
    {
       mVSomVois[aKS]->flag_set_kth_false(mFlagVois);
    }
    mVSomVois.clear();
    mCurArc = 0;
}


void cAppli_GenTriplet::GenTriplet()
{
   for (int aKS=0 ; aKS<int(mVecS.size()) ; aKS++)
   {
       std::cout << "ONE SOOMM  GT " << mVecS.size() - aKS << " \n";
       for( tItAGT itA=mVecS[aKS]->begin(mSubAll) ; itA.go_on() ; itA++)
       {
             GenTriplet(*itA);
       }
   }
}

bool cAppli_GenTriplet::AddTriplet(tSomGT & aS1,tSomGT & aS2,tSomGT & aS3)
{
   cTripletInt aTr(aS1.attr().Num(),aS2.attr().Num(),aS3.attr().Num());
   if (mTriplets.find(aTr) != mTriplets.end())
      return false;

   mTriplets.insert(aTr);
   
   return true;
}


void  AddPackToMerge(CamStenope * aCS1,CamStenope * aCS2,const ElPackHomologue & aPack,cFixedMergeStruct<2,Pt2df> &   aMap,int aInd0)
{
    for (ElPackHomologue::const_iterator itP=aPack.begin(); itP!=aPack.end() ; itP++)
    {
        Pt2dr aP1 = aCS1->F2toPtDirRayonL3(itP->P1());
        Pt2dr aP2 = aCS2->F2toPtDirRayonL3(itP->P2());
        Pt2df aQ1(aP1.x,aP1.y);
        Pt2df aQ2(aP2.x,aP2.y);
        // if (aSwap) ElSwap(aQ1,aQ2);
        aMap.AddArc(aQ1,aInd0,aQ2,1-aInd0);
    }
}

cAppli_GenTriplet::cAppli_GenTriplet(int argc,char ** argv) :
    mGrT      (),
    mFlagVois (mGrT.alloc_flag_som()),
    mShow     (true)
{

   ElInitArgMain
   (
        argc,argv,
        LArgMain() <<  EAMC(mFullName,"Pattern"),
        LArgMain() << EAM(mNameOriCalib,"OriCalib",true,"Orientation for calibration ")
                   << EAM(mShow,"Show",true,"Show intermediar message ")
   );

   mEASF.Init(mFullName);
   mNM = new cNewO_NameManager(mEASF.mDir,mNameOriCalib,"dat");

   cInterfChantierNameManipulateur::tSet  aVIm = *(mEASF.SetIm());
   std::sort(aVIm.begin(),aVIm.end());
   for (int aKIm=0 ; aKIm<int(aVIm.size());  aKIm++)
   {
        const std::string & aName = (aVIm)[aKIm];
        tSomGT & aS = mGrT.new_som(cGTrip_AttrSom(aKIm,aName,*this));
        mVecS.push_back(&aS);
        mMapS[aName] = &aS;
   }

   const cInterfChantierNameManipulateur::tSet *  aSetCple =  mEASF.mICNM->Get("NKS-Set-CplIm2OriRel@@dmp");
   std::string aKeyCple2I = "NKS-Assoc-CplIm2OriRel@@dmp";
   for (int aKC=0 ; aKC<int(aSetCple->size());  aKC++)
   {
        std::pair<std::string,std::string> aPair = mEASF.mICNM->Assoc2To1(aKeyCple2I,(*aSetCple)[aKC],false);
        std::string aN1 = aPair.first;
        std::string aN2 = aPair.second;
        tSomGT * aS1 = mMapS[aN1];
        tSomGT * aS2 = mMapS[aN2];
        if (aS1 && aS2)
        {
           if (!  mGrT.arc_s1s2(*aS1,*aS2))
           {
               cXml_Ori2Im aXmlO = StdGetFromSI(mEASF.mDir+(*aSetCple)[aKC],Xml_Ori2Im);
               if (aXmlO.Geom().IsInit())
               {
                  CamStenope * aCS1 = aS1->attr().Im().CS();
                  CamStenope * aCS2 = aS2->attr().Im().CS();
                  ElPackHomologue  aPck1 = mNM->PackOfName(aN1,aN2);
                  ElPackHomologue  aPck2 = mNM->PackOfName(aN2,aN1);

                  cFixedMergeStruct<2,Pt2df>   aMap;
                  AddPackToMerge(aCS1,aCS2,aPck1,aMap,0);
                  AddPackToMerge(aCS2,aCS1,aPck2,aMap,1);
                  aMap.DoExport();
                  const  std::list<cFixedMergeTieP<2,Pt2df> *> &  aLM = aMap.ListMerged();
                  int aNbSym = 0;
                  for (std::list<cFixedMergeTieP<2,Pt2df> *>::const_iterator itM = aLM.begin() ; itM!=aLM.end() ; itM++)
                  {
                      const cFixedMergeTieP<2,Pt2df> & aM = **itM;
                      if (aM.NbArc() ==2) aNbSym++;
                  }
                  

                  const cXml_O2IRotation & aXO = aXmlO.Geom().Val().Ori();
                  ElRotation3D aR(aXO.Centre(),ImportMat(aXO.Ori()),true);
                  cGTrip_AttrASym * anASym  =  new  cGTrip_AttrASym(aXmlO);
                  anASym->VP1().reserve(aNbSym);
                  anASym->VP2().reserve(aNbSym);

                  Pt2df aInfP1(1e20,1e20);
                  Pt2df aSupP1(-1e20,-1e20);
                  for (std::list<cFixedMergeTieP<2,Pt2df> *>::const_iterator itM = aLM.begin() ; itM!=aLM.end() ; itM++)
                  {
                      const cFixedMergeTieP<2,Pt2df> & aM = **itM;
                      if (aM.NbArc() ==2)
                      {
                         Pt2df aP1 = aM.GetVal(0);
                         Pt2df aP2 = aM.GetVal(1);
                         anASym->VP1().push_back(aP1);
                         anASym->VP2().push_back(aP2);
                         aInfP1 = Inf(aInfP1,aP1);
                         aSupP1 = Sup(aSupP1,aP1);
                         // SetSup(aSupP1,aP1);
                      }
                  }
                  anASym->InfP1() = aInfP1;
                  anASym->SupP1() = aSupP1;

                  



                  mGrT.add_arc(*aS1,*aS2,cGTrip_AttrArc(aR,anASym,true),cGTrip_AttrArc(aR.inv(),anASym,false));
                  aMap.Delete();
               }

           }
        }
        if (((aKC%10)==0) && mShow)
        {
           std::cout << "AAAAAAAAAAAA " << aSetCple->size() - aKC << "\n";
        }
   }
    // getchar();
}


int GenTriplet_main(int argc,char ** argv)
{
   cAppli_GenTriplet anAppli(argc,argv);
   anAppli.GenTriplet();
   return EXIT_SUCCESS;
}


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
Footer-MicMac-eLiSe-25/06/2007*/
