## Theory Notes

### Background

We are considering the recurrent group of the Abelian Sandpile Model (ASM).
A configuration is a graph $G$ (with max-degree 4), with integer vertex labels.
Let $G'$ be the graph where each vertex is connected to a sink, with multiplicity $4 - deg(u)$.
Consider $L$ the induced Laplacian of $G'$ excluding the sink.

Now, fixing a topology for $G$, we have an ASM.
We can write each configuration as a vector of labels, and we have the vector-addition lattice.
We quotient out by the zero-lattice, i.e. of the form $Lx$ for some $x$.
The recurrent elements, i.e. of the form ${3, ..., 3} \oplus x$, for some configuration $x$, are a group.

Algorithmically, we define the stabilize function $T$ that maps a config to its stable representation, i.e. all elements in ${0, 1, 2, 3}$.
Then, we can prove that the identity element is $T(6 - T(6))$.
This is the initial implementation given in the starter code.

Note that computing $T$ takes quadratic time in the number of vertices in the worst case.
This is an upper-bound, because each element can only be "toppled" $O(n)$ times (suppose all topples are at the end; each topple subtracts $4$ from the label, but the label is bounded by the sum of in the starting config, i.e. $O(n)$).
With the Pacman, for $N = 1001$, then $n = N^2$ and we have a quartic time algorithm.

### Verification

It is important to know how to (quickly) verify where something is the identity element $e$.
For example, even if $xy = x$ for some $x, y$, it may not be the case that $y$ is the identity.
As I change the algorithms, being able to cerify correctness is important.

As described in [@doman2020identity], the identity element is the unique stable configuration that satisfies two properties:

- it is recurrent (i.e. in the group, of the form $3 \oplus x$)
- it is null (i.e. it is of the form $Ly$ for some $y$).

If I change the algorithm, returning these certificates will be important.

## Planned steps

0. compare to using Halide
1. using smaller-width elements
2. vertical symmetry
3. worklist in stabilize
4. reducing the number of branches
5. OpenMP parallel
6. divide-and-conquer for cache oblivious
7. Hilbert curve (maybe??)
8. burning algorithm
9. exponential burning algoithm

## References

```
@phdthesis{doman2020identity,
  title={The Identity of the Abelian Sandpile Group},
  author={Doman, Noah},
  year={2020}
}
```

```
@inproceedings{shibazaki2015efficient,
  title={Efficient computation method for identity element of Abelian sandpile model},
  author={Shibazaki, Takumi and Namiki, Takao},
  booktitle={2015 Third International Symposium on Computing and Networking (CANDAR)},
  pages={430--435},
  year={2015},
  organization={IEEE}
}
```
