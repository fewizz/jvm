package java.util;

import java.io.Serializable;
import java.lang.reflect.Array;

public class ArrayList<E> extends AbstractList<E>
	implements List<E>, RandomAccess, Cloneable, Serializable
{

	Object[] storage_;
	int size_ = 0;

	public ArrayList(int initialCapacity) {
		this.storage_ = new Object[initialCapacity];
	}

	public ArrayList() {
		this.storage_ = new Object[10];
	}

	public ArrayList(Collection<? extends E> c) {
		this.storage_ = new Object[c.size()];
		addAll(c);
	}

	public void trimToSize() {
		if(size_ < storage_.length) {
			Object[] new_storage = new Object[size_];
			System.arraycopy(storage_, 0, new_storage, 0, size_);
		}
	}

	public void ensureCapacity(int minCapacity) {
		if(storage_.length < minCapacity) {
			Object[] new_storage = new Object[minCapacity];
			System.arraycopy(storage_, 0, new_storage, 0, minCapacity);
		}
	}

	@Override
	public int size() {
		return size_;
	}

	@Override
	public boolean isEmpty() {
		return size_ == 0;
	}

	@Override
	public boolean contains(Object o) {
		return indexOf(o) >= 0;
	}

	@Override
	public int indexOf(Object o) {
		for(int x = 0; x < size_; ++x) {
			if(Objects.equals(storage_[x], o)) {
				return x;
			}
		}
		return -1;
	}

	@Override
	public int lastIndexOf(Object o) {
		for(int x = size_; x > 0;) {
			--x;
			if(Objects.equals(storage_[x], o)) {
				return x;
			}
		}
		return -1;
	}

	public Object clone() {
		var cloned = new ArrayList<>(storage_.length);
		System.arraycopy(storage_, 0, cloned.storage_, 0, size_);
		return cloned;
	}

	public Object[] toArray() {
		Object[] a = new Object[size_];
		System.arraycopy(storage_, 0, a, 0, size_);
		return a;
	}

}