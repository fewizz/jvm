package jvm.internal;

import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;

public class UTF8Charset extends Charset {

	public static final UTF8Charset INSTANCE = new UTF8Charset();

	private UTF8Charset() {
		super(
			"UTF-8", //canonicalName
			null     // aliases
		);
	}

	@Override
	public boolean contains(Charset cs) {
		return false; // TODO
	}

	@Override
	public CharsetDecoder newDecoder() {
		return UTF8Decoder.INSTANCE;
	}

	@Override
	public CharsetEncoder newEncoder() {
		return UTF8Encoder.INSTANCE;
	}


}