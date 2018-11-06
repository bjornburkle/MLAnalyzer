import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
#options.register('skipEvents', 
#    default=0, 
#    mult=VarParsing.VarParsing.multiplicity.singleton,
#    mytype=VarParsing.VarParsing.varType.int,
#    info = "skipEvents")
# TODO: put this option in cmsRun scripts
options.register('processMode', 
    default='JetLevel', 
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.string,
    info = "process mode: JetLevel or EventLevel")
#options.register('maxEvents',
#    default=-1,
#    mult=VarParsing.VarParsing.multiplicity.singleton,
#    mytype=VarParsing.VarParsing.varType.int,
#    info = 'Number of events processed')
options.register('UseAK8',
    #default=False,
    default=1,
    mult=VarParsing.VarParsing.multiplicity.singleton,
    mytype=VarParsing.VarParsing.varType.int,
    info = 'Whether or not you use AK8 jets: True or False')
options.parseArguments()

process = cms.Process("FEVTAnalyzer")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
#process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
#process.load("Geometry.CMSCommonData.cmsIdealGeometryXML_cfi");
#process.load("Geometry.CaloEventSetup.CaloGeometry_cfi");
#process.load("Geometry.CaloEventSetup.CaloTopology_cfi");
process.GlobalTag.globaltag = cms.string('80X_dataRun2_HLT_v12')
process.es_prefer_GlobalTag = cms.ESPrefer('PoolDBESSource','GlobalTag')

process.maxEvents = cms.untracked.PSet( 
    #input = cms.untracked.int32(options.maxEvents) 
    input = cms.untracked.int32(-1) 
    #input = cms.untracked.int32(1000000) 
    )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
      #'test'
      options.inputFiles
      )
    #, skipEvents = cms.untracked.uint32(options.skipEvents)
    , skipEvents = cms.untracked.uint32(0)
    )
print " >> Loaded",len(options.inputFiles),"input files from list."

process.load("MLAnalyzer.RecHitAnalyzer.RHAnalyzer_cfi")
process.fevt.mode = cms.string(options.processMode)

if options.UseAK8:
    process.fevt.pfJetCollection = cms.InputTag('ak8PFJetsCHS')
    process.fevt.genJetCollection = cms.InputTag('ak8GenJets')
else:
    process.fevt.pfJetCollection = cms.InputTag('ak4PFJets')
    process.fevt.genJetCollection = cms.InputTag('ak4GenJets')
   
#process.fevt.mode = cms.string("JetLevel") # for when using crab
#process.fevt.mode = cms.string("EventLevel") # for when using crab
print " >> Processing as:",(process.fevt.mode)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
    #fileName = cms.string('/uscms/home/bburkle/nobackup/working_area/CMSSW_9_3_0/src/MLAnalyzer/output/test/QCD_test.root')
    )

#process.SimpleMemoryCheck = cms.Service( "SimpleMemoryCheck", ignoreTotal = cms.untracked.int32(1) )
process.p = cms.Path(process.fevt)
