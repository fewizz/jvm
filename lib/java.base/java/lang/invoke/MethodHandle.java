package java.lang.invoke;

import java.lang.constant.Constable;

public abstract class MethodHandle implements Constable {
	private long class_;
	private long member_;
	private byte kind_;

	private MethodHandle() {}

}