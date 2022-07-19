package java.nio;

public final class ByteOrder {

	private final String name_;

	private ByteOrder(String name) {
		this.name_ = name;
	}

	public static final ByteOrder
		BIG_ENDIAN = new ByteOrder("BIG_ENDIAN"),
		LITTLE_ENDIAN = new ByteOrder("LITTLE_ENDIAN");

	public static native ByteOrder nativeOrder();
}