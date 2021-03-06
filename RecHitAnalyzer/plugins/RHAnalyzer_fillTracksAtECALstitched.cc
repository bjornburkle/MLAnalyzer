#include "MLAnalyzer/RecHitAnalyzer/interface/RecHitAnalyzer.h"

// Fill Tracks into stitched EEm_EB_EEp image //////////////////////
// Store all Track positions into a stitched EEm_EB_EEp image 

TH2F *hEvt_EE_tracksPt[nEE];
TH2F *hEvt_EE_tracksQPt[nEE];
TProfile2D *hECAL_tracks;
TProfile2D *hECAL_tracksPt;
TProfile2D *hECAL_tracksQPt;
std::vector<float> vECAL_tracksPt_;
std::vector<float> vECAL_tracksQPt_;

// Initialize branches _______________________________________________________________//
void RecHitAnalyzer::branchesTracksAtECALstitched ( TTree* tree, edm::Service<TFileService> &fs ) {

  // Branches for images
  tree->Branch("ECAL_tracksPt",    &vECAL_tracksPt_);
  tree->Branch("ECAL_tracksQPt",    &vECAL_tracksQPt_);

  // Intermediate helper histogram (single event only)
  hEvt_EE_tracksPt[0] = new TH2F("evt_EEm_tracksPt", "E(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX, -TMath::Pi(), TMath::Pi(),
      5*(HBHE_IETA_MAX_HE-1-HBHE_IETA_MAX_EB), eta_bins_EEm );
  hEvt_EE_tracksPt[1] = new TH2F("evt_EEp_tracksPt", "E(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX, -TMath::Pi(), TMath::Pi(),
      5*(HBHE_IETA_MAX_HE-1-HBHE_IETA_MAX_EB), eta_bins_EEp );

  hEvt_EE_tracksQPt[0] = new TH2F("evt_EEm_tracksQPt", "qxPt(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX, -TMath::Pi(), TMath::Pi(),
      5*(HBHE_IETA_MAX_HE-1-HBHE_IETA_MAX_EB), eta_bins_EEm );
  hEvt_EE_tracksQPt[1] = new TH2F("evt_EEp_tracksQPt", "qxPt(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX, -TMath::Pi(), TMath::Pi(),
      5*(HBHE_IETA_MAX_HE-1-HBHE_IETA_MAX_EB), eta_bins_EEp );


  // Histograms for monitoring
  hECAL_tracks = fs->make<TProfile2D>("ECAL_tracks", "E(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX,    EB_IPHI_MIN-1, EB_IPHI_MAX,
      2*ECAL_IETA_MAX_EXT, -ECAL_IETA_MAX_EXT,   ECAL_IETA_MAX_EXT );

  hECAL_tracksPt = fs->make<TProfile2D>("ECAL_tracksPt", "E(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX,    EB_IPHI_MIN-1, EB_IPHI_MAX,
      2*ECAL_IETA_MAX_EXT, -ECAL_IETA_MAX_EXT,   ECAL_IETA_MAX_EXT );

  hECAL_tracksQPt = fs->make<TProfile2D>("ECAL_tracksQPt", "qxPt(i#phi,i#eta);i#phi;i#eta",
      EB_IPHI_MAX,    EB_IPHI_MIN-1, EB_IPHI_MAX,
      2*ECAL_IETA_MAX_EXT, -ECAL_IETA_MAX_EXT,   ECAL_IETA_MAX_EXT );

} // branchesTracksAtECALstitched()

// Function to map EE(phi,eta) histograms to ECAL(iphi,ieta) vector _______________________________//
void fillTracksAtECAL_with_EEproj ( TH2F *hEvt_EE_tracksPt_, TH2F *hEvt_EE_tracksQPt_, int ieta_global_offset, int ieta_signed_offset ) {

  int ieta_global_, ieta_signed_;
  int ieta_, iphi_, idx_;
  float trackPt_;
  float trackQPt_;

  for (int ieta = 1; ieta < hEvt_EE_tracksPt_->GetNbinsY()+1; ieta++) {
    ieta_ = ieta - 1;
    ieta_global_ = ieta_ + ieta_global_offset;
    ieta_signed_ = ieta_ + ieta_signed_offset;
    for (int iphi = 1; iphi < hEvt_EE_tracksPt_->GetNbinsX()+1; iphi++) {

      trackPt_ = hEvt_EE_tracksPt_->GetBinContent( iphi, ieta );
      trackQPt_ = hEvt_EE_tracksQPt_->GetBinContent( iphi, ieta );
      if ( (trackPt_ <= zs) ) continue;
      // NOTE: EB iphi = 1 does not correspond to physical phi = -pi so need to shift!
      iphi_ = iphi  + 5*38; // shift
      iphi_ = iphi_ > EB_IPHI_MAX ? iphi_-EB_IPHI_MAX : iphi_; // wrap-around
      iphi_ = iphi_ - 1;
      idx_  = ieta_global_*EB_IPHI_MAX + iphi_;
      // Fill vector for image
      vECAL_tracksPt_[idx_] = trackPt_;
      vECAL_tracksQPt_[idx_] = trackQPt_;
      // Fill histogram for monitoring
      hECAL_tracks->Fill( iphi_, ieta_signed_, 1. );
      hECAL_tracksPt->Fill( iphi_, ieta_signed_, trackPt_ );
      hECAL_tracksQPt->Fill( iphi_, ieta_signed_, trackQPt_ );

    } // iphi_
  } // ieta_

} // fillTracksAtECAL_with_EEproj

// Fill stitched EE-, EB, EE+ rechits ________________________________________________________//
void RecHitAnalyzer::fillTracksAtECALstitched ( const edm::Event& iEvent, const edm::EventSetup& iSetup ) {

  int iphi_, ieta_, iz_, idx_;
  int ieta_global, ieta_signed;
  int ieta_global_offset, ieta_signed_offset;
  float eta, phi, trackPt_, trackQPt_;
  GlobalPoint pos;

  vECAL_tracksPt_.assign( 2*ECAL_IETA_MAX_EXT*EB_IPHI_MAX, 0. );
  vECAL_tracksQPt_.assign( 2*ECAL_IETA_MAX_EXT*EB_IPHI_MAX, 0. );
  for ( int iz(0); iz < nEE; ++iz ) hEvt_EE_tracksPt[iz]->Reset();
  for ( int iz(0); iz < nEE; ++iz ) hEvt_EE_tracksQPt[iz]->Reset();

  edm::Handle<EcalRecHitCollection> EBRecHitsH_;
  iEvent.getByToken( EBRecHitCollectionT_, EBRecHitsH_ );
  edm::Handle<EcalRecHitCollection> EERecHitsH_;
  iEvent.getByToken( EERecHitCollectionT_, EERecHitsH_ );
  edm::ESHandle<CaloGeometry> caloGeomH_;
  iSetup.get<CaloGeometryRecord>().get( caloGeomH_ );
  const CaloGeometry* caloGeom = caloGeomH_.product();

  edm::Handle<reco::TrackCollection> tracksH_;
  iEvent.getByToken( trackCollectionT_, tracksH_ );

  reco::Track::TrackQuality tkQt_ = reco::Track::qualityByName("highPurity");

  for ( reco::TrackCollection::const_iterator iTk = tracksH_->begin();
        iTk != tracksH_->end(); ++iTk ) {
    if ( !(iTk->quality(tkQt_)) ) continue;
    eta = iTk->eta();
    phi = iTk->phi();
    if ( std::abs(eta) > 3. ) continue;
    DetId id( spr::findDetIdECAL( caloGeom, eta, phi, false ) );
    if ( id.subdetId() == EcalBarrel ) continue;
    if ( id.subdetId() == EcalEndcap ) {
      iz_ = (eta > 0.) ? 1 : 0;
      // Fill intermediate helper histogram by eta,phi
      hEvt_EE_tracksPt[iz_]->Fill( phi, eta, iTk->pt() );
      hEvt_EE_tracksQPt[iz_]->Fill( phi, eta, iTk->charge()*iTk->pt() );
    }
  } // tracks


  // Map EE-(phi,eta) to bottom part of ECAL(iphi,ieta)
  ieta_global_offset = 0;
  ieta_signed_offset = -ECAL_IETA_MAX_EXT;
  fillTracksAtECAL_with_EEproj( hEvt_EE_tracksPt[0], hEvt_EE_tracksQPt[0], ieta_global_offset, ieta_signed_offset );

  // Fill middle part of ECAL(iphi,ieta) with the EB rechits.
  ieta_global_offset = 55;

  for ( reco::TrackCollection::const_iterator iTk = tracksH_->begin();
        iTk != tracksH_->end(); ++iTk ) { 
    if ( !(iTk->quality(tkQt_)) ) continue;
    eta = iTk->eta();
    phi = iTk->phi();
    trackPt_ = iTk->pt();
    trackQPt_ = (iTk->charge()*iTk->pt());
    if ( std::abs(eta) > 3. ) continue;
    DetId id( spr::findDetIdECAL( caloGeom, eta, phi, false ) );
    if ( id.subdetId() == EcalEndcap ) continue;
    if ( id.subdetId() == EcalBarrel ) { 
      EBDetId ebId( id );
      iphi_ = ebId.iphi() - 1;
      ieta_ = ebId.ieta() > 0 ? ebId.ieta()-1 : ebId.ieta();
      if ( trackPt_ <= zs ) continue;
      // Fill vector for image
      ieta_signed = ieta_;
      ieta_global = ieta_ + EB_IETA_MAX + ieta_global_offset;
      idx_ = ieta_global*EB_IPHI_MAX + iphi_; 
      vECAL_tracksPt_[idx_] += trackPt_;
      vECAL_tracksQPt_[idx_] += trackQPt_;
      // Fill histogram for monitoring
      hECAL_tracks->Fill( iphi_, ieta_signed, 1. );
      hECAL_tracksPt->Fill( iphi_, ieta_signed, trackPt_ );
      hECAL_tracksQPt->Fill( iphi_, ieta_signed, trackQPt_ );
    }

  } // EB Tracks



  // Map EE+(phi,eta) to upper part of ECAL(iphi,ieta)
  ieta_global_offset = ECAL_IETA_MAX_EXT + EB_IETA_MAX;
  ieta_signed_offset = EB_IETA_MAX;
  fillTracksAtECAL_with_EEproj( hEvt_EE_tracksPt[1], hEvt_EE_tracksQPt[1], ieta_global_offset, ieta_signed_offset );

} // fillTracksAtECALstitched()
