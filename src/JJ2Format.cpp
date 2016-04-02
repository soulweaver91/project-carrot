#include "JJ2Format.h"
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDataStream>

QByteArray JJ2Format::fromLittleEndian(QByteArray le) {
    QDataStream a(le);
    QByteArray res;
    a.setByteOrder(QDataStream::BigEndian);
    qint8 byte;
    while (!a.atEnd()) {
        a >> byte;
        res.prepend(byte);
    }
    return res;
}

// "borrowed" code from OpenMPT

// AM(FF) stuff

struct AMFFRiffChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idRIFF	= 0x46464952,
		idAMFF	= 0x46464D41,
		idAM__	= 0x20204D41,
		idMAIN	= 0x4E49414D,
		idINIT	= 0x54494E49,
		idORDR	= 0x5244524F,
		idPATT	= 0x54544150,
		idINST	= 0x54534E49,
		idSAMP	= 0x504D4153,
		idAI__	= 0x20204941,
		idAS__	= 0x20205341,
	};

	typedef ChunkIdentifiers id_type;

	quint32 id;		// See ChunkIdentifiers
	quint32 length;	// Chunk size without header <- save as already swapped int
};


// This header is used for both AM's "INIT" as well as AMFF's "MAIN" chunk
struct AMFFMainChunk
{
	// Main Chunk flags
	enum MainFlags
	{
		amigaSlides = 0x01,
	};

	char   songname[64];
	quint8  flags;
	quint8  channels;
	quint8  speed;
	quint8  tempo;
	quint32 unknown;		// 0x16078035 if original file was MOD, 0xC50100FF for everything else? it's 0xFF00FFFF in Carrotus.j2b (AMFF version)
	quint8  globalvolume;
};


// AMFF instrument envelope point (old format)
struct AMFFEnvelopePoint
{
	quint16 tick;
	quint8  value;	// 0...64
};

/*
// AMFF instrument envelope (old format)
struct AMFFEnvelope
{
	// Envelope flags (also used for RIFF AM)
	enum EnvelopeFlags
	{
		envEnabled	= 0x01,
		envSustain	= 0x02,
		envLoop		= 0x04,
	};

	quint8  envFlags;			// high nibble = pan env flags, low nibble = vol env flags (both nibbles work the same way)
	quint8  envNumPoints;		// high nibble = pan env length, low nibble = vol env length
	quint8  envSustainPoints;	// you guessed it... high nibble = pan env sustain point, low nibble = vol env sustain point
	quint8  envLoopStarts;		// i guess you know the pattern now.
	quint8  envLoopEnds;			// same here.
	AMFFEnvelopePoint volEnv[10];
	AMFFEnvelopePoint panEnv[10]; // convert these to BE on encounter


	// Convert weird envelope data to OpenMPT's internal format.
	void ConvertEnvelope(quint8 flags, quint8 numPoints, quint8 sustainPoint, quint8 loopStart, quint8 loopEnd, const AMFFEnvelopePoint *points, InstrumentEnvelope &mptEnv) const {
		mptEnv.dwFlags.set(ENV_ENABLED, (flags & AMFFEnvelope::envEnabled) != 0);
		mptEnv.dwFlags.set(ENV_SUSTAIN, (flags & AMFFEnvelope::envSustain) && mptEnv.nSustainStart <= mptEnv.nNodes);
		mptEnv.dwFlags.set(ENV_LOOP, (flags & AMFFEnvelope::envLoop) && mptEnv.nLoopStart <= mptEnv.nLoopEnd && mptEnv.nLoopStart <= mptEnv.nNodes);

		// The buggy mod2j2b converter will actually NOT limit this to 10 points if the envelope is longer.
		mptEnv.nNodes = Util::Min(numPoints, static_cast<quint8>(10));

		mptEnv.nSustainStart = mptEnv.nSustainEnd = sustainPoint;

		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;

		for(size_t i = 0; i < 10; i++)
		{
			mptEnv.Ticks[i] = points[i].tick >> 4;
			if(i == 0)
				mptEnv.Ticks[0] = 0;
			else if(mptEnv.Ticks[i] < mptEnv.Ticks[i - 1])
				mptEnv.Ticks[i] = mptEnv.Ticks[i - 1] + 1;

			mptEnv.Values[i] = Clamp(points[i].value, uint8(0), uint8(0x40));
		}
	}

	void ConvertToMPT(ModInstrument &mptIns) const
	{
		// interleaved envelope data... meh. gotta split it up here and decode it separately.
		// note: mod2j2b is BUGGY and always writes ($original_num_points & 0x0F) in the header,
		// but just has room for 10 envelope points. That means that long (>= 16 points)
		// envelopes are cut off, and envelopes have to be trimmed to 10 points, even if
		// the header claims that they are longer.
		ConvertEnvelope(envFlags & 0x0F, envNumPoints & 0x0F, envSustainPoints & 0x0F, envLoopStarts & 0x0F, envLoopEnds & 0x0F, volEnv, mptIns.VolEnv);
		ConvertEnvelope(envFlags >> 4, envNumPoints >> 4, envSustainPoints >> 4, envLoopStarts >> 4, envLoopEnds >> 4, panEnv, mptIns.PanEnv);
	}
};


// AMFF instrument header (old format)
struct AMFFInstrumentHeader
{
	quint8  unknown;				// 0x00
	quint8  index;				// actual instrument number
	char   name[28];
	quint8  numSamples;
	quint8  sampleMap[120];
	quint8  vibratoType; // this and below need conversion
	quint16 vibratoSweep;
	quint16 vibratoDepth;
	quint16 vibratoRate;
	AMFFEnvelope envelopes;
	quint16 fadeout;

	// Convert instrument data to OpenMPT's internal format.
	void ConvertToMPT(ModInstrument &mptIns, SAMPLEINDEX baseSample)
	{
		StringFixer::ReadString<StringFixer::maybeNullTerminated>(mptIns.name, name);

		for(size_t i = 0; i < CountOf(sampleMap); i++)
		{
			mptIns.Keyboard[i] = sampleMap[i] + baseSample + 1;
		}

		mptIns.nFadeOut = fadeout << 5;
		envelopes.ConvertToMPT(mptIns);
	}

};

// AMFF sample header (old format)
struct AMFFSampleHeader
{
	// Sample flags (also used for RIFF AM)
	enum SampleFlags
	{
		smp16Bit	= 0x04,
		smpLoop		= 0x08,
		smpPingPong	= 0x10,
		smpPanning	= 0x20,
		smpExists	= 0x80,
		// some flags are still missing... what is e.g. 0x8000?
	};

	quint32 id;	// "SAMP"
	quint32 chunkSize;	// header + sample size
	char   name[28];
	quint8  pan;
	quint8  volume;
	quint16 flags;
	quint32 length;
	quint32 loopStart;
	quint32 loopEnd;
	quint32 sampleRate;
	quint32 reserved1;
	quint32 reserved2;
    
	// Convert sample header to OpenMPT's internal format.
	void ConvertToMPT(AMFFInstrumentHeader &instrHeader, ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nPan = pan * 4;
		mptSmp.nVolume = volume * 4;
		mptSmp.nGlobalVol = 64;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nC5Speed = sampleRate;

		if(instrHeader.vibratoType < CountOf(j2bAutoVibratoTrans))
			mptSmp.nVibType = j2bAutoVibratoTrans[instrHeader.vibratoType];
		mptSmp.nVibSweep = static_cast<uint8>(instrHeader.vibratoSweep);
		mptSmp.nVibRate = static_cast<uint8>(instrHeader.vibratoRate / 16);
		mptSmp.nVibDepth = static_cast<uint8>(instrHeader.vibratoDepth / 4);
		if((mptSmp.nVibRate | mptSmp.nVibDepth) != 0)
		{
			// Convert XM-style vibrato sweep to IT
			mptSmp.nVibSweep = 255 - mptSmp.nVibSweep;
		}

		if(flags & AMFFSampleHeader::smp16Bit)
			mptSmp.uFlags |= CHN_16BIT;
		if(flags & AMFFSampleHeader::smpLoop)
			mptSmp.uFlags |= CHN_LOOP;
		if(flags & AMFFSampleHeader::smpPingPong)
			mptSmp.uFlags |= CHN_PINGPONGLOOP;
		if(flags & AMFFSampleHeader::smpPanning)
			mptSmp.uFlags |= CHN_PANNING;
	}

	// Retrieve the internal sample format flags for this sample.
	SampleIO GetSampleFormat() const
	{
		return SampleIO(
			(flags & AMFFSampleHeader::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM);
	}
};


// AM instrument envelope point (new format)
struct AMEnvelopePoint
{
	quint16 tick;
	quint16 value;
};


// AM instrument envelope (new format)
struct AMEnvelope
{
	quint16 flags;
	quint8  numPoints;	// actually, it's num. points - 1, and 0xFF if there is no envelope
	quint8  sustainPoint;
	quint8  loopStart;
	quint8  loopEnd;
	AMEnvelopePoint values[10];
	quint16 fadeout;		// why is this here? it's only needed for the volume envelope...

	// Convert envelope data to OpenMPT's internal format.
	void ConvertToMPT(InstrumentEnvelope &mptEnv, enmEnvelopeTypes envType) const
	{
		if(numPoints == 0xFF || numPoints == 0)
			return;

		mptEnv.dwFlags.set(ENV_ENABLED, (flags & AMFFEnvelope::envEnabled) != 0);
		mptEnv.dwFlags.set(ENV_SUSTAIN, (flags & AMFFEnvelope::envSustain) && mptEnv.nSustainStart <= mptEnv.nNodes);
		mptEnv.dwFlags.set(ENV_LOOP, (flags & AMFFEnvelope::envLoop) && mptEnv.nLoopStart <= mptEnv.nLoopEnd && mptEnv.nLoopStart <= mptEnv.nNodes);

		mptEnv.nNodes = Util::Min(numPoints + 1, 10);

		mptEnv.nSustainStart = mptEnv.nSustainEnd = sustainPoint;

		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;

		for(size_t i = 0; i < 10; i++)
		{
			mptEnv.Ticks[i] = values[i].tick >> 4;
			if(i == 0)
				mptEnv.Ticks[i] = 0;
			else if(mptEnv.Ticks[i] < mptEnv.Ticks[i - 1])
				mptEnv.Ticks[i] = mptEnv.Ticks[i - 1] + 1;

			const quint16 val = values[i].value;
			switch(envType)
			{
			case ENV_VOLUME:	// 0....32767
				mptEnv.Values[i] = (BYTE)((val + 1) >> 9);
				break;
			case ENV_PITCH:		// -4096....4096
				mptEnv.Values[i] = (BYTE)((((int16)val) + 0x1001) >> 7);
				break;
			case ENV_PANNING:	// -32768...32767
				mptEnv.Values[i] = (BYTE)((((int16)val) + 0x8001) >> 10);
				break;
			}
			Limit(mptEnv.Values[i], BYTE(ENVELOPE_MIN), BYTE(ENVELOPE_MAX));
		}
	}
};

// Convert OpenMPT's internal envelope format into an IT/MPTM envelope.
void ITEnvelope::ConvertToIT(const InstrumentEnvelope &mptEnv, BYTE envOffset, BYTE envDefault)
//---------------------------------------------------------------------------------------------
{
	// Envelope Flags
	if(mptEnv.dwFlags[ENV_ENABLED]) flags |= ITEnvelope::envEnabled;
	if(mptEnv.dwFlags[ENV_LOOP]) flags |= ITEnvelope::envLoop;
	if(mptEnv.dwFlags[ENV_SUSTAIN]) flags |= ITEnvelope::envSustain;
	if(mptEnv.dwFlags[ENV_CARRY]) flags |= ITEnvelope::envCarry;

	// Nodes and Loops
	num = (uint8)min(mptEnv.nNodes, 25);
	lpb = (uint8)mptEnv.nLoopStart;
	lpe = (uint8)mptEnv.nLoopEnd;
	slb = (uint8)mptEnv.nSustainStart;
	sle = (uint8)mptEnv.nSustainEnd;

	// Envelope Data
	if(mptEnv.nNodes > 0)
	{
		// Attention: Full MPTM envelope is stored in extended instrument properties
		for(size_t ev = 0; ev < 25; ev++)
		{
			data[ev * 3] = mptEnv.Values[ev] - envOffset;
			data[ev * 3 + 1] = mptEnv.Ticks[ev] & 0xFF;
			data[ev * 3 + 2] = mptEnv.Ticks[ev] >> 8;
		}
	} else
	{
		// Fix non-existing envelopes so that they can still be edited in Impulse Tracker.
		num = 2;
		MemsetZero(data);
		data[0] = data[3] = envDefault - envOffset;
		data[4] = 10;
	}
}


// AM instrument header (new format)
struct AMInstrumentHeader
{
	quint32 headSize;	// Header size (i.e. the size of this struct)
	quint8  unknown1;	// 0x00
	quint8  index;		// Actual instrument number
	char   name[32];
	quint8  sampleMap[128];
	quint8  vibratoType;
	quint16 vibratoSweep;
	quint16 vibratoDepth;
	quint16 vibratoRate;
	quint8  unknown2[7];
	AMEnvelope volEnv;
	AMEnvelope pitchEnv;
	AMEnvelope panEnv;
	quint16 numSamples;

	// Convert instrument data to OpenMPT's internal format.
	void ConvertToMPT(ModInstrument &mptIns, SAMPLEINDEX baseSample)
	{
		StringFixer::ReadString<StringFixer::maybeNullTerminated>(mptIns.name, name);

		STATIC_ASSERT(CountOf(sampleMap) <= CountOf(mptIns.Keyboard));
		for(BYTE i = 0; i < CountOf(sampleMap); i++)
		{
			mptIns.Keyboard[i] = sampleMap[i] + baseSample + 1;
		}

		mptIns.nFadeOut = volEnv.fadeout << 5;

		volEnv.ConvertToMPT(mptIns.VolEnv, ENV_VOLUME);
		pitchEnv.ConvertToMPT(mptIns.PitchEnv, ENV_PITCH);
		panEnv.ConvertToMPT(mptIns.PanEnv, ENV_PANNING);

		if(numSamples == 0)
		{
			MemsetZero(mptIns.Keyboard);
		}
	}
};


// Convert OpenMPT's internal instrument representation to an ITInstrument.
size_t ITInstrument::ConvertToIT(const ModInstrument &mptIns, bool compatExport, const CSoundFile &sndFile)
//---------------------------------------------------------------------------------------------------------
{
	MemsetZero(*this);

	// Header
	id = ITInstrument::magic;
	trkvers = 0x0214;

	StringFixer::WriteString<StringFixer::nullTerminated>(filename, mptIns.filename);
	StringFixer::WriteString<StringFixer::nullTerminated>(name, mptIns.name);

	// Volume / Panning
	fadeout = static_cast<uint16>(min(mptIns.nFadeOut >> 5, 256));
	gbv = static_cast<uint8>(min(mptIns.nGlobalVol * 2, 128));
	dfp = static_cast<uint8>(min(mptIns.nPan / 4, 64));
	if(!mptIns.dwFlags[INS_SETPANNING]) dfp |= ITInstrument::ignorePanning;

	// Random Variation
	rv = min(mptIns.nVolSwing, 100);
	rp = min(mptIns.nPanSwing, 64);

	// NNA Stuff
	nna = mptIns.nNNA;
	dct = (mptIns.nDCT < DCT_PLUGIN || !compatExport) ? mptIns.nDCT : DCT_NONE;
	dca = mptIns.nDNA;

	// Pitch / Pan Separation
	pps = mptIns.nPPS;
	ppc = mptIns.nPPC;

	// Filter Stuff
	ifc = mptIns.GetCutoff() | (mptIns.IsCutoffEnabled() ? ITInstrument::enableCutoff : 0x00);
	ifr = mptIns.GetResonance() | (mptIns.IsResonanceEnabled() ? ITInstrument::enableResonance : 0x00);

	// MIDI Setup
	mbank = mptIns.wMidiBank;
	mpr = mptIns.nMidiProgram;
	if(mptIns.nMidiChannel || mptIns.nMixPlug == 0 || compatExport)
	{
		// Default. Prefer MIDI channel over mixplug to keep the semantics intact.
		mch = mptIns.nMidiChannel;
	} else
	{
		// Keep compatibility with MPT 1.16's instrument format if possible, as XMPlay / BASS also uses this.
		mch = mptIns.nMixPlug + 128;
	}

	// Sample Map
	nos = 0;
	vector<bool> smpcount(sndFile.GetNumSamples(), false);
	for(size_t i = 0; i < 120; i++)
	{
		keyboard[i * 2] = (mptIns.NoteMap[i] >= NOTE_MIN && mptIns.NoteMap[i] <= NOTE_MAX) ? (mptIns.NoteMap[i] - NOTE_MIN) : static_cast<uint8>(i);

		const SAMPLEINDEX smp = mptIns.Keyboard[i];
		if(smp < MAX_SAMPLES && smp < 256)
		{
			keyboard[i * 2 + 1] = static_cast<uint8>(smp);

			if(smp && smp <= sndFile.GetNumSamples() && !smpcount[smp - 1])
			{
				// We haven't considered this sample yet. Update number of samples.
				smpcount[smp - 1] = true;
				nos++;
			}
		}
	}

	// Writing Volume envelope
	volenv.ConvertToIT(mptIns.VolEnv, 0, 64);
	// Writing Panning envelope
	panenv.ConvertToIT(mptIns.PanEnv, 32, 32);
	// Writing Pitch Envelope
	pitchenv.ConvertToIT(mptIns.PitchEnv, 32, 32);
	if(mptIns.PitchEnv.dwFlags[ENV_FILTER]) pitchenv.flags |= ITEnvelope::envFilter;

	return sizeof(ITInstrument);
}


// AM sample header (new format)
struct AMSampleHeader
{
	quint32 headSize;	// Header size (i.e. the size of this struct), apparently not including headSize.
	char   name[32];
	quint16 pan;
	quint16 volume;
	quint16 flags;
	quint16 unknown;		// 0x0000 / 0x0080?
	quint32 length;
	quint32 loopStart;
	quint32 loopEnd;
	quint32 sampleRate;

	// Convert sample header to OpenMPT's internal format.
	void ConvertToMPT(AMInstrumentHeader &instrHeader, ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nPan = Util::Min(pan, static_cast<uint16>(32767)) * 256 / 32767;
		mptSmp.nVolume = Util::Min(volume, static_cast<uint16>(32767)) * 256 / 32767;
		mptSmp.nGlobalVol = 64;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nC5Speed = sampleRate;

		if(instrHeader.vibratoType < CountOf(j2bAutoVibratoTrans))
			mptSmp.nVibType = j2bAutoVibratoTrans[instrHeader.vibratoType];
		mptSmp.nVibSweep = static_cast<uint8>(instrHeader.vibratoSweep);
		mptSmp.nVibRate = static_cast<uint8>(instrHeader.vibratoRate / 16);
		mptSmp.nVibDepth = static_cast<uint8>(instrHeader.vibratoDepth / 4);
		if((mptSmp.nVibRate | mptSmp.nVibDepth) != 0)
		{
			// Convert XM-style vibrato sweep to IT
			mptSmp.nVibSweep = 255 - mptSmp.nVibSweep;
		}

		if(flags & AMFFSampleHeader::smp16Bit)
			mptSmp.uFlags |= CHN_16BIT;
		if(flags & AMFFSampleHeader::smpLoop)
			mptSmp.uFlags |= CHN_LOOP;
		if(flags & AMFFSampleHeader::smpPingPong)
			mptSmp.uFlags |= CHN_PINGPONGLOOP;
		if(flags & AMFFSampleHeader::smpPanning)
			mptSmp.uFlags |= CHN_PANNING;
	}

	// Retrieve the internal sample format flags for this sample.
	SampleIO GetSampleFormat() const
	{
		return SampleIO(
			(flags & AMFFSampleHeader::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM);
	}
};


// Convert OpenMPT's internal sample representation to an ITSample.
void ITSample::ConvertToIT(const ModSample &mptSmp, MODTYPE fromType)
//-------------------------------------------------------------------
{
	MemsetZero(*this);

	// Header
	id = ITSample::magic;

	StringFixer::WriteString<StringFixer::nullTerminated>(filename, mptSmp.filename);
	//StringFixer::WriteString<StringFixer::nullTerminated>(name, m_szNames[nsmp]);

	// Volume / Panning
	gvl = static_cast<BYTE>(mptSmp.nGlobalVol);
	vol = static_cast<BYTE>(mptSmp.nVolume / 4);
	dfp = static_cast<BYTE>(mptSmp.nPan / 4);
	if(mptSmp.uFlags & CHN_PANNING) dfp |= ITSample::enablePanning;

	// Sample Format / Loop Flags
	if(mptSmp.nLength && mptSmp.pSample)
	{
		flags = ITSample::sampleDataPresent;
		if(mptSmp.uFlags & CHN_LOOP) flags |= ITSample::sampleLoop;
		if(mptSmp.uFlags & CHN_SUSTAINLOOP) flags |= ITSample::sampleSustain;
		if(mptSmp.uFlags & CHN_PINGPONGLOOP) flags |= ITSample::sampleBidiLoop;
		if(mptSmp.uFlags & CHN_PINGPONGSUSTAIN) flags |= ITSample::sampleBidiSustain;

		if(mptSmp.uFlags & CHN_STEREO)
		{
			flags |= ITSample::sampleStereo;
		}
		if(mptSmp.uFlags & CHN_16BIT)
		{
			flags |= ITSample::sample16Bit;
		}
		cvt = ITSample::cvtSignedSample;
	} else
	{
		flags = 0x00;
	}

	// Frequency
	C5Speed = mptSmp.nC5Speed ? mptSmp.nC5Speed : 8363;

	// Size and loops
	length = mptSmp.nLength;
	loopbegin = mptSmp.nLoopStart;
	loopend = mptSmp.nLoopEnd;
	susloopbegin = mptSmp.nSustainStart;
	susloopend = mptSmp.nSustainEnd;

	// Auto Vibrato settings
	static const uint8 autovibxm2it[8] = { 0, 2, 4, 1, 3, 0, 0, 0 };	// OpenMPT VibratoType -> IT Vibrato
	vit = autovibxm2it[mptSmp.nVibType & 7];
	vis = min(mptSmp.nVibRate, 64);
	vid = min(mptSmp.nVibDepth, 32);
	vir = min(mptSmp.nVibSweep, 255);

	if((vid | vis) != 0 && (fromType & MOD_TYPE_XM))
	{
		// Sweep is upside down in XM
		vir = 255 - vir;
	}
}


// Convert RIFF AM(FF) pattern data to MPT pattern data.
bool ConvertAMPattern(FileReader chunk, PATTERNINDEX pat, bool isAM, CSoundFile &sndFile)
//---------------------------------------------------------------------------------------
{
	// Effect translation LUT
	static const ModCommand::COMMAND amEffTrans[] =
	{
		CMD_ARPEGGIO, CMD_PORTAMENTOUP, CMD_PORTAMENTODOWN, CMD_TONEPORTAMENTO,
		CMD_VIBRATO, CMD_TONEPORTAVOL, CMD_VIBRATOVOL, CMD_TREMOLO,
		CMD_PANNING8, CMD_OFFSET, CMD_VOLUMESLIDE, CMD_POSITIONJUMP,
		CMD_VOLUME, CMD_PATTERNBREAK, CMD_MODCMDEX, CMD_TEMPO,
		CMD_GLOBALVOLUME, CMD_GLOBALVOLSLIDE, CMD_KEYOFF, CMD_SETENVPOSITION,
		CMD_CHANNELVOLUME, CMD_CHANNELVOLSLIDE, CMD_PANNINGSLIDE, CMD_RETRIG,
		CMD_TREMOR, CMD_XFINEPORTAUPDOWN,
	};

	enum
	{
		rowDone		= 0,		// Advance to next row
		channelMask	= 0x1F,		// Mask for retrieving channel information
		volFlag		= 0x20,		// Volume effect present
		noteFlag	= 0x40,		// Note + instr present
		effectFlag	= 0x80,		// Effect information present
		dataFlag	= 0xE0,		// Channel data present
	};

	if(!chunk.BytesLeft())
	{
		return false;
	}

	ROWINDEX numRows = Clamp(static_cast<ROWINDEX>(chunk.ReadUint8()) + 1, ROWINDEX(1), MAX_PATTERN_ROWS);

	if(sndFile.Patterns.Insert(pat, numRows))
		return false;

	const CHANNELINDEX channels = sndFile.GetNumChannels();
	if(channels == 0)
		return false;

	PatternRow rowBase = sndFile.Patterns[pat].GetRow(0);
	ROWINDEX row = 0;

	while(row < numRows && chunk.BytesLeft())
	{
		const quint8 flags = chunk.ReadUint8();

		if(flags == rowDone)
		{
			row++;
			rowBase = sndFile.Patterns[pat].GetRow(row);
			continue;
		}

		ModCommand &m = rowBase[min((flags & channelMask), channels - 1)];

		if(flags & dataFlag)
		{
			if(flags & effectFlag) // effect
			{
				m.param = chunk.ReadUint8();
				m.command = chunk.ReadUint8();

				if(m.command < CountOf(amEffTrans))
				{
					// command translation
					m.command = amEffTrans[m.command];
				} else
				{
#ifdef DEBUG
					CHAR s[64];
					wsprintf(s, "J2B: Unknown command: 0x%X, param 0x%X", m.command, m.param);
					Log(s);
#endif // DEBUG
					m.command = CMD_NONE;
				}

				// Handling special commands
				switch(m.command)
				{
				case CMD_ARPEGGIO:
					if(m.param == 0) m.command = CMD_NONE;
					break;
				case CMD_VOLUME:
					if(m.volcmd == VOLCMD_NONE)
					{
						m.volcmd = VOLCMD_VOLUME;
						m.vol = Clamp(m.param, BYTE(0), BYTE(64));
						m.command = CMD_NONE;
						m.param = 0;
					}
					break;
				case CMD_TONEPORTAVOL:
				case CMD_VIBRATOVOL:
				case CMD_VOLUMESLIDE:
				case CMD_GLOBALVOLSLIDE:
				case CMD_PANNINGSLIDE:
					if (m.param & 0xF0) m.param &= 0xF0;
					break;
				case CMD_PANNING8:
					if(m.param <= 0x80) m.param = min(m.param << 1, 0xFF);
					else if(m.param == 0xA4) {m.command = CMD_S3MCMDEX; m.param = 0x91;}
					break;
				case CMD_PATTERNBREAK:
					m.param = ((m.param >> 4) * 10) + (m.param & 0x0F);
					break;
				case CMD_MODCMDEX:
					m.ExtendedMODtoS3MEffect();
					break;
				case CMD_TEMPO:
					if(m.param <= 0x1F) m.command = CMD_SPEED;
					break;
				case CMD_XFINEPORTAUPDOWN:
					switch(m.param & 0xF0)
					{
					case 0x10:
						m.command = CMD_PORTAMENTOUP;
						break;
					case 0x20:
						m.command = CMD_PORTAMENTODOWN;
						break;
					}
					m.param = (m.param & 0x0F) | 0xE0;
					break;
				}
			}

			if (flags & noteFlag) // note + ins
			{
				m.instr = chunk.ReadUint8();
				m.note = chunk.ReadUint8();
				if(m.note == 0x80) m.note = NOTE_KEYOFF;
				else if(m.note > 0x80) m.note = NOTE_FADE;	// I guess the support for IT "note fade" notes was not intended in mod2j2b, but hey, it works! :-D
			}

			if (flags & volFlag) // volume
			{
				m.volcmd = VOLCMD_VOLUME;
				m.vol = chunk.ReadUint8();
				if(isAM)
				{
					m.vol = m.vol * 64 / 127;
				}
			}
		}
	}

	return true;
}


bool CSoundFile::ReadAM(FileReader &file)
//---------------------------------------
{
	file.Rewind();
	AMFFRiffChunk fileHeader;
	if(!file.ReadConvertEndianness(fileHeader))
	{
		return false;
	}

	if(fileHeader.id != AMFFRiffChunk::idRIFF)
	{
		return false;
	}

	bool isAM; // false: AMFF, true: AM

	uint32 format = file.ReadUint32LE();
	if(format == AMFFRiffChunk::idAMFF)
		isAM = false; // "AMFF"
	else if(format == AMFFRiffChunk::idAM__)
		isAM = true; // "AM  "
	else
		return false;

	m_nChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;

	ChunkReader chunkFile(file);
	// RIFF AM has a padding byte so that all chunks have an even size.
	ChunkReader::ChunkList<AMFFRiffChunk> chunks = chunkFile.ReadChunks<AMFFRiffChunk>(isAM ? 2 : 1);

	// "MAIN" - Song info (AMFF)
	// "INIT" - Song info (AM)
	FileReader chunk(chunks.GetChunk(isAM ? AMFFRiffChunk::idINIT : AMFFRiffChunk::idMAIN));
	AMFFMainChunk mainChunk;
	if(!chunk.IsValid() || !chunk.Read(mainChunk))
	{
		return false;
	}

	StringFixer::ReadString<StringFixer::maybeNullTerminated>(m_szNames[0], mainChunk.songname);

	if(mainChunk.channels < 1 || !chunk.CanRead(mainChunk.channels))
	{
		return false;
	}
	m_SongFlags = SONG_ITOLDEFFECTS | SONG_ITCOMPATGXX;
	m_SongFlags.set(SONG_LINEARSLIDES, !(mainChunk.flags & AMFFMainChunk::amigaSlides));

	m_nChannels = min(mainChunk.channels, MAX_BASECHANNELS);
	m_nDefaultSpeed = mainChunk.speed;
	m_nDefaultTempo = mainChunk.tempo;
	m_nDefaultGlobalVolume = mainChunk.globalvolume * 2;
	m_nSamplePreAmp = m_nVSTiVolume = 48;
	m_nType = MOD_TYPE_J2B;

	ASSERT(mainChunk.unknown == LittleEndian(0xFF0001C5) || mainChunk.unknown == LittleEndian(0x35800716) || mainChunk.unknown == LittleEndian(0xFF00FFFF));

	// It seems like there's no way to differentiate between
	// Muted and Surround channels (they're all 0xA0) - might
	// be a limitation in mod2j2b.
	for(CHANNELINDEX nChn = 0; nChn < m_nChannels; nChn++)
	{
		ChnSettings[nChn].nVolume = 64;
		ChnSettings[nChn].nPan = 128;

		uint8 pan = chunk.ReadUint8();

		if(isAM)
		{
			if(pan > 128)
				ChnSettings[nChn].dwFlags = CHN_MUTE;
			else
				ChnSettings[nChn].nPan = pan * 2;
		} else
		{
			if(pan >= 128)
				ChnSettings[nChn].dwFlags = CHN_MUTE;
			else
				ChnSettings[nChn].nPan = pan * 4;
		}
	}

	if(chunks.ChunkExists(AMFFRiffChunk::idORDR))
	{
		// "ORDR" - Order list
		FileReader chunk(chunks.GetChunk(AMFFRiffChunk::idORDR));
		uint8 numOrders = chunk.ReadUint8() + 1;
		Order.ReadAsByte(chunk, numOrders);
	}

	// "PATT" - Pattern data for one pattern
	vector<FileReader> pattChunks = chunks.GetAllChunks(AMFFRiffChunk::idPATT);
	for(vector<FileReader>::iterator patternIter = pattChunks.begin(); patternIter != pattChunks.end(); patternIter++)
	{
		FileReader chunk(*patternIter);
		PATTERNINDEX pat = chunk.ReadUint8();
		size_t patternSize = chunk.ReadUint32LE();
		ConvertAMPattern(chunk.GetChunk(patternSize), pat, isAM, *this);
	}

	if(!isAM)
	{
		// "INST" - Instrument (only in RIFF AMFF)
		vector<FileReader> instChunks = chunks.GetAllChunks(AMFFRiffChunk::idINST);
		for(vector<FileReader>::iterator instIter = instChunks.begin(); instIter != instChunks.end(); instIter++)
		{
			FileReader chunk(*instIter);
			AMFFInstrumentHeader instrHeader;
			if(!chunk.ReadConvertEndianness(instrHeader))
			{
				continue;
			}

			const INSTRUMENTINDEX instr = instrHeader.index + 1;
			if(instr >= MAX_INSTRUMENTS)
				continue;

			ModInstrument *pIns = AllocateInstrument(instr);
			if(pIns == nullptr)
			{
				continue;
			}

			m_nInstruments = max(m_nInstruments, instr);

			instrHeader.ConvertToMPT(*pIns, m_nSamples);

			// read sample sub-chunks - this is a rather "flat" format compared to RIFF AM and has no nested RIFF chunks.
			for(size_t samples = 0; samples < instrHeader.numSamples; samples++)
			{
				AMFFSampleHeader sampleHeader;

				if(m_nSamples + 1 >= MAX_SAMPLES || !chunk.ReadConvertEndianness(sampleHeader))
				{
					continue;
				}

				const SAMPLEINDEX smp = ++m_nSamples;

				if(sampleHeader.id != AMFFRiffChunk::idSAMP)
				{
					continue;
				}

				StringFixer::ReadString<StringFixer::maybeNullTerminated>(m_szNames[smp], sampleHeader.name);
				sampleHeader.ConvertToMPT(instrHeader, Samples[smp]);
				sampleHeader.GetSampleFormat().ReadSample(Samples[smp], chunk);
			}
		}
	} else
	{
		// "RIFF" - Instrument (only in RIFF AM)
		vector<FileReader> instChunks = chunks.GetAllChunks(AMFFRiffChunk::idRIFF);
		for(vector<FileReader>::iterator instIter = instChunks.begin(); instIter != instChunks.end(); instIter++)
		{
			ChunkReader chunk(*instIter);
			if(chunk.ReadUint32LE() != AMFFRiffChunk::idAI__)
			{
				continue;
			}

			AMFFRiffChunk instChunk;
			if(!chunk.ReadConvertEndianness(instChunk) || instChunk.id != AMFFRiffChunk::idINST)
			{
				continue;
			}

			AMInstrumentHeader instrHeader;
			if(!chunk.ReadConvertEndianness(instrHeader))
			{
				continue;
			}
			ASSERT(instrHeader.headSize + 4 == sizeof(instrHeader));

			const INSTRUMENTINDEX instr = instrHeader.index + 1;
			if(instr >= MAX_INSTRUMENTS)
				continue;

			ModInstrument *pIns = AllocateInstrument(instr);
			if(pIns == nullptr)
			{
				continue;
			}
			m_nInstruments = max(m_nInstruments, instr);

			instrHeader.ConvertToMPT(*pIns, m_nSamples);

			// Read sample sub-chunks (RIFF nesting ftw)
			ChunkReader::ChunkList<AMFFRiffChunk> sampleChunkFile = chunk.ReadChunks<AMFFRiffChunk>(2);
			vector<FileReader> sampleChunks = sampleChunkFile.GetAllChunks(AMFFRiffChunk::idRIFF);
			ASSERT(sampleChunks.size() == instrHeader.numSamples);

			for(vector<FileReader>::iterator smpIter = sampleChunks.begin(); smpIter != sampleChunks.end(); smpIter++)
			{
				ChunkReader sampleChunk(*smpIter);

				if(sampleChunk.ReadUint32LE() != AMFFRiffChunk::idAS__ || m_nSamples + 1 >= MAX_SAMPLES)
				{
					continue;
				}

				// Don't read more samples than the instrument header claims to have.
				if((instrHeader.numSamples--) == 0)
				{
					break;
				}

				const SAMPLEINDEX smp = ++m_nSamples;

				// Aaand even more nested chunks! Great, innit?
				AMFFRiffChunk sampleHeaderChunk;
				if(!sampleChunk.ReadConvertEndianness(sampleHeaderChunk) || sampleHeaderChunk.id != AMFFRiffChunk::idSAMP)
				{
					break;
				}

				FileReader sampleFileChunk = sampleChunk.GetChunk(sampleHeaderChunk.length);

				AMSampleHeader sampleHeader;
				if(!sampleFileChunk.ReadConvertEndianness(sampleHeader))
				{
					break;
				}

				StringFixer::ReadString<StringFixer::maybeNullTerminated>(m_szNames[smp], sampleHeader.name);

				sampleHeader.ConvertToMPT(instrHeader, Samples[smp]);

				sampleFileChunk.Seek(sampleHeader.headSize + 4);
				sampleHeader.GetSampleFormat().ReadSample(Samples[smp], sampleFileChunk);
			}
		
		}
	}

	return true;
}



// Convert OpenMPT's internal instrument representation to an ITInstrumentEx. Returns amount of bytes that need to be written to file.
size_t ITInstrumentEx::ConvertToIT(const ModInstrument &mptIns, bool compatExport, const CSoundFile &sndFile)
//-----------------------------------------------------------------------------------------------------------
{
	size_t instSize = iti.ConvertToIT(mptIns, compatExport, sndFile);

	if(compatExport)
	{
		return instSize;
	}

	// Sample Map
	bool usedExtension = false;
	iti.nos = 0;
	vector<bool>smpcount(sndFile.GetNumSamples(), false);
	for(size_t i = 0; i < 120; i++)
	{
		const SAMPLEINDEX smp = mptIns.Keyboard[i];
		if(smp < MAX_SAMPLES)
		{
			if(smp >= 256)
			{
				// We need to save the upper byte for this sample index.
				iti.keyboard[i * 2 + 1] = static_cast<uint8>(smp & 0xFF);
				keyboardhi[i] = static_cast<uint8>(smp >> 8);
				usedExtension = true;
			}

			if(smp && smp <= sndFile.GetNumSamples() && !smpcount[smp - 1])
			{
				// We haven't considered this sample yet. Update number of samples.
				smpcount[smp - 1] = true;
				iti.nos++;
			}
		}
	}

	if(usedExtension)
	{
		// If we actually had to extend the sample map, update the magic bytes and instrument size.
		iti.dummy = ITInstrumentEx::mptx;
		instSize = sizeof(ITInstrumentEx);
	}

	return instSize;
}


*/


// END


QString JJ2Format::convertJ2B(const QString& original) {
    QFile outfile(original);
    QFile infile(QString("RunTimeConverted/converted.it"));
    if (!outfile.exists()) {
        return "";
    }
    if (!outfile.open(QIODevice::ReadOnly)) {
        return "";
    }

    // http://mods.jazz2online.com/j2b.txt
    // J2B header structure
    //     4 bytes, magic string "MUSE"
    //     4 bytes, magic number (little endian):
    //             0xDEADBABE (AMFF)
    //         or
    //             0xDEADBEAF (AM)
    //         By selective testing, it appears that J2B files from TSF (and most probably CC) use AM
    //         while the ones from the original version use AMFF.
    //     4 bytes, filesize
    //     4 bytes, CRC
    //     4 bytes, compressed data size
    //     4 bytes, uncompressed data size
    // qUncompress wants the last size so don't remove that, and we need the magic number to know
    // whether the data is AM or AMFF; skip CRC and sizes and assume the file is not corrupt
    if (outfile.read(4).toInt() != 0x4553554D) {
        outfile.close();
        return "";
    }

    int format = outfile.read(4).toInt();
    outfile.seek(8);

    QByteArray size_bytes = fromLittleEndian(outfile.read(4));    
    QByteArray data = qUncompress(size_bytes + outfile.readAll());
    
    outfile.close();
    if (!infile.open(QIODevice::WriteOnly)) {
        return "";
    }

    // rest is todo
    if (format == 0xAFBEADDE) {
        
    } else if (format == 0xBEBAADDE) {

    } else {
        // unknown format
    }

    // handle somehow the situation if the file already exists
    return "RunTimeConverted/converted.it";
}
