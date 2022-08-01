package jvm.internal;

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
		return null;
	}
	
}