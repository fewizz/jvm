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

	@Override
	public Object clone() {
		var cloned = new ArrayList<>(storage_.length);
		System.arraycopy(storage_, 0, cloned.storage_, 0, size_);
		return cloned;
	}

	@Override
	public Object[] toArray() {
		Object[] a = new Object[size_];
		System.arraycopy(storage_, 0, a, 0, size_);
		return a;
	}

	@SuppressWarnings("unchecked")
	@Override
	public <T> T[] toArray(T[] a) {
		if(a.length < size_) {
			a = (T[]) Array.newInstance(a.getClass().getComponentType(), size_);
		}
		System.arraycopy(storage_, 0, a, 0, size_);
		for(int x = size_; x < a.length; ++x) {
			a[x] = null;
		}
		return a;
	}

	@SuppressWarnings("unchecked")
	@Override
	public E get(int index) {
		if(index >= size_) {
			throw new UnsupportedOperationException();
		}
		return (E) storage_[index];
	}

	@SuppressWarnings("unchecked")
	@Override
	public E set(int index, E element) {
		E prev = (E) storage_[index];
		storage_[index] = element;
		++modCount;
		return prev;
	}

	private void grow(int min) {
		int newSize = storage_.length +
			Math.max(min - storage_.length, storage_.length >> 2);
		
		var newStorage = new Object[newSize];
		System.arraycopy(storage_, 0, newStorage, 0, size_);
	}

	@Override
	public boolean add(E e) {
		if(size_ == storage_.length) {
			grow(storage_.length + 1);
		}
		storage_[size_++] = e;
		++modCount;
		return true;
	}

	@Override
	public void add(int index, E element) {
		if(size_ == storage_.length) {
			grow(storage_.length + 1);
		}
		Object[] right = new Object[size_ - index]; // TODO we can do better
		System.arraycopy(storage_, index, right, 0, right.length);
		storage_[index] = element;
		System.arraycopy(right, 0, storage_, index + 1, right.length);
		++size_;
		++modCount;
	}

	@SuppressWarnings("unchecked")
	@Override
	public E remove(int index) {
		E prev = (E) storage_[index];
		Object[] right = new Object[size_ - (index + 1)]; // TODO we can do better
		System.arraycopy(storage_, index + 1, right, 0, right.length);
		System.arraycopy(right, 0, storage_, index, right.length);
		--size_;
		++modCount;
		return prev;
	}

	@Override
	public boolean remove(Object o) {
		int i = indexOf(o);
		if(i == -1) {
			return false;
		}
		remove(i);
		return true;
	}

	public void clear() {
		size_ = 0;
		for(int x = 0; x < 0; ++x) {
			storage_[x] = null;
		}
	}

}