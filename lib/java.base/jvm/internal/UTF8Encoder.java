package jvm.internal;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

public final class UTF8Encoder extends CharsetEncoder {

	public static final UTF8Encoder INSTANCE = new UTF8Encoder();

	protected UTF8Encoder() {
		super(
			UTF8Charset.INSTANCE,
			1.0F, // averageBytesPerChar
			4.0F  // maxBytesPerChar
		);
	}

	@Override
	protected CoderResult encodeLoop(CharBuffer in, ByteBuffer out) {
		while(true) {
			if(in.remaining() == 0) {
				return CoderResult.UNDERFLOW;
			}

			char surrs[] = new char[Math.min(in.remaining(), 2)];
			int surrs_count = 0;
			in.get(in.position(), surrs);

			int cp;

			char c0 = surrs[0];
			if(c0 < 0xD800) {
				cp = c0;
				surrs_count = 1;
			} else if(c0 <= 0xDBFF) {
				if(surrs.length < 2) {
					return CoderResult.UNDERFLOW;
				}

				char c1 = surrs[1];
				if(c1 >= 0xDC00 && c1 <= 0xDFFF) {
					int h = c0;
					h -= 0xD800;
					h <<= 10;

					int l = c1;
					l -= 0xDC00;

					cp = (h | l) + 0x10000;
					surrs_count = 2;
				} else {
					return CoderResult.MALFORMED_2;
				}
			} else {
				return CoderResult.MALFORMED_1;
			}

			if((cp >>> 7) == 0) {
				if(out.remaining() < 1) {
					return CoderResult.OVERFLOW;
				}
				out.put((byte)cp);
			}
			else if((cp >>> (6 + 5)) == 0) {
				if(out.remaining() < 2) {
					return CoderResult.OVERFLOW;
				}
				out.put((byte)((0b110 << 5) | (cp >>> 6)));
				out.put((byte)((0b10  << 6) | (cp & 0b111111)));
			}
			else if((cp >>> (6 + 6 + 4)) == 0) {
				if(out.remaining() < 3) {
					return CoderResult.OVERFLOW;
				}
				out.put((byte)((0b1110 << 4) | ((cp >>> (6 + 6)) & 0b1111  )));
				out.put((byte)((0b10   << 6) | ((cp >>>  6)      & 0b111111)));
				out.put((byte)((0b10   << 6) | ( cp              & 0b111111)));
			}
			else {//if((cp >> (6 + 6 + 6 + 3)) == 0) {
				if(out.remaining() < 4) {
					return CoderResult.OVERFLOW;
				}
				out.put(
					(byte)((0b11110 << 3) | ((cp >>> (6 + 6 + 6)) & 0b111   ))
				);
				out.put(
					(byte)((0b10    << 6) | ((cp >>> (6 + 6))     & 0b111111))
				);
				out.put(
					(byte)((0b10    << 6) | ((cp >>>  6)          & 0b111111))
				);
				out.put(
					(byte)((0b10    << 6) | ( cp                  & 0b111111))
				);
			}
			//else {
			//	CoderResult.MALFORMED_2; // shouldn't happen
			//}
			in.position(in.position() + surrs_count);
		}
	}
	
}
