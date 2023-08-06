package jvm;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;

public final class UTF8Decoder extends CharsetDecoder {

	public static final UTF8Decoder INSTANCE = new UTF8Decoder();

	private UTF8Decoder() {
		super(
			UTF8Charset.INSTANCE,
			1.0F, // averageCharsPerByte,
			1.0F // maxCharsPerByte
		);
	}

	@Override
	protected CoderResult decodeLoop(ByteBuffer in, CharBuffer out) {
		while(true) {
			if(in.remaining() == 0) {
				return CoderResult.UNDERFLOW;
			}

			byte units[] = new byte[Math.min(in.remaining(), 4)];
			int units_count = 0;
			in.get(in.position(), units);

			int cp;

			byte b0 = units[0];

			if((b0 >>> 7) == 0) {
				cp = b0;
				units_count = 1;
			} else {
				if(units.length < 2) {
					return CoderResult.UNDERFLOW;
				}

				byte b1 = units[1];

				if((b0 >>> 5) == 0b110) {
					cp =
						(b0 & 0b11111 ) << 6 |
						(b1 & 0b111111);
					units_count = 2;
				} else {
					if(units.length < 3) {
						return CoderResult.UNDERFLOW;
					}

					byte b2 = units[2];

					if((b0 >>> 4) == 0b1110) {
						cp =
							(b0 & 0b1111  ) << (6 + 6) |
							(b1 & 0b111111) <<  6 |
							(b2 & 0b111111);
						units_count = 3;
					} else {
						if(units.length < 4) {
							return CoderResult.UNDERFLOW;
						}

						byte b3 = units[3];

						if((b0 >>> 3) == 0b11110) {
							cp =
								(b0 & 0b111   ) << (6 + 6 + 6) |
								(b1 & 0b111111) << (6 + 6) |
								(b2 & 0b111111) <<  6 |
								(b3 & 0b111111);
							units_count = 4;
						}
						else {
							return CoderResult.MALFORMED_4;
						}
					}
				}
			}
			if(cp > 0xFFFF) {
				if(out.remaining() < 2) {
					return CoderResult.OVERFLOW;
				}
				cp -= 0x10000;
				out.put((char)(0xD800 + (cp >>> 10)));
				out.put((char)(0xDC00 + (cp & 0b1111111111)));
			}
			else {
				if(out.remaining() < 1) {
					return CoderResult.OVERFLOW;
				}
				out.put((char)cp);
			}
			in.position(in.position() + units_count);
		}
	}
	
}