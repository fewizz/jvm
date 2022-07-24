package java.util;

public abstract class AbstractList<E>
	extends AbstractCollection<E>
	implements List<E>
{
	protected transient int modCount;

	protected AbstractList() {
		super();
	}

	@Override
	public boolean add(E e) {
		add(size(), e);
		return true;
	}

	@Override
	public abstract E get(int index);

	@Override
	public E set(int index, E element) {
		throw new UnsupportedOperationException();
	}

	@Override
	public void add(int index, E element) {
		throw new UnsupportedOperationException();
	}

	@Override
	public E remove(int index) {
		throw new UnsupportedOperationException();
	}

	@Override
	public int indexOf(Object o) {
		ListIterator<E> it = listIterator();
		while(it.hasNext()) {
			if(Objects.equals(it.next(), o)) {
				return it.previousIndex();
			}
		}
		return -1;
	}

	@Override
	public int lastIndexOf(Object o) {
		ListIterator<E> it = listIterator(size());
		while(it.hasPrevious()) {
			if(Objects.equals(it.previous(), o)) {
				return it.nextIndex();
			}
		}
		return -1;
	}

	@Override
	public void clear() {
		removeRange(0, size());
	}

	@Override
	public boolean addAll(int index, Collection<? extends E> c) {
		boolean changed = false;
		for(E e : c) {
			add(index++, e);
			changed = true;
		}
		return changed;
	}

	@Override
	public Iterator<E> iterator() {
		return new __Iterator();
	}

	@Override
	public ListIterator<E> listIterator() {
		return listIterator(0);
	}

	@Override
	public ListIterator<E> listIterator(int index) {
		return new __ListIterator();
	}

	@Override
	public List<E> subList(int fromIndex, int toIndex) {
		return null; // TODO
	}

	@Override
	public boolean equals(Object o) {
		if(this == o) {
			return true;
		}
		if(o instanceof AbstractList) {
			Iterator<E> it0 = iterator();
			Iterator<?> it1 = ((AbstractList<?>) o).iterator();

			while(true) {
				boolean hn0 = it0.hasNext();
				boolean hn1 = it1.hasNext();

				if(hn0 != hn1) {
					return false;
				}

				if(!hn0 && !hn1) {
					return true;
				}

				if(!Objects.equals(it0.next(), it1.next())) {
					return false;
				}
			}
		}

		return true;
	}

	@Override
	public int hashCode() {
		int hash = 1;
		for (E e : this) {
			hash = 31 * hash + (e == null ? 0 : e.hashCode());
		}
		return hash;
	}

	protected void removeRange(int fromIndex, int toIndex) {
		
	}

	class __Iterator implements Iterator<E> {

		int index_ = 0;
		int prevIndex_ = -1;
		int mods_ = modCount;

		protected void checkCM() {
			if(modCount != mods_) {
				throw new ConcurrentModificationException();
			}
		}

		@Override
		public boolean hasNext() {
			return index_ < size();
		}

		@Override
		public E next() {
			checkCM();
			E result = null;
			NullPointerException npe = null;
			try {
				result = get(index_);
				checkCM();
				prevIndex_ = index_;
				++index_;
			} catch(NullPointerException e) {
				npe = e;
			} finally {
				checkCM();
				if(npe != null) {
					throw new NoSuchElementException(npe);
				}
			}

			return result;
		}

		@Override
		public void remove() {
			if(prevIndex_ == -1) {
				throw new IllegalStateException();
			}
			checkCM();

			AbstractList.this.remove(prevIndex_);
			// prev call was next(), not previous()
			if(prevIndex_ < index_) {
				--index_;
			}
			prevIndex_ = -1;
			mods_ = modCount;
		}

	};

	class __ListIterator extends __Iterator implements ListIterator<E> {

		@Override
		public void add(E e) {
			if(prevIndex_ == -1) {
				throw new IllegalStateException();
			}
			checkCM();

			AbstractList.this.add(index_, e);
			++index_;
			prevIndex_ = -1;
			mods_ = modCount;
		}

		@Override
		public boolean hasPrevious() {
			return index_ > 0;
		}

		@Override
		public int nextIndex() {
			return index_;
		}

		@Override
		public E previous() {
			checkCM();
			E result = null;
			NullPointerException npe = null;

			try {
				result = AbstractList.this.get(index_ - 1);
				--index_;
				prevIndex_ = index_;
			} catch(NullPointerException e) {
				npe = e;
			} finally {
				checkCM();
				if(npe != null) {
					throw new NoSuchElementException(npe);
				}
			}

			return result;
		}

		@Override
		public int previousIndex() {
			return index_ - 1;
		}

		@Override
		public void set(E e) {
			if(prevIndex_ == -1) {
				throw new IllegalStateException();
			}
			checkCM();
			AbstractList.this.set(index_, e);
			mods_ = modCount;
		}

	}

}