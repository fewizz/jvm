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
		return null;
	}
	
}
