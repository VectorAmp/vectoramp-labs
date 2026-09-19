<div align="center">
  <a href="https://vectoramp.com/">
    <picture>
      <source media="(prefers-color-scheme: light)" srcset="https://vectoramp.com/logo-full-light.svg">
      <source media="(prefers-color-scheme: dark)" srcset="https://vectoramp.com/logo-full-dark.svg">
      <img alt="VectorAmp Logo" src="https://vectoramp.com/logo-full-dark.svg" width="50%">
    </picture>
  </a>
</div>

# VectorAmp Labs

Explore vector search through reproducible code, benchmarks, notebooks, and
technical demonstrations from VectorAmp Engineering.

This repository accompanies our public technical writing. Each lab is designed
to be understandable and reproducible in isolation, with its methodology,
requirements, raw results, and limitations documented alongside the code.

> [!IMPORTANT]
> VectorAmp Labs contains educational experiments and reference implementations.
> It is not the production SABLE implementation, and examples may prioritize
> clarity and reproducibility over production integration.

## Labs

| Lab | Subject | Status |
| --- | --- | --- |
| [AVX-512 vector distance](articles/avx512-vector-distance/) | Dense squared-L2 and PQ asymmetric-distance kernels | Draft |

## Reproducibility

Every lab should include:

- build and execution instructions;
- correctness checks against a scalar implementation;
- hardware, compiler, and runtime metadata;
- benchmark methodology and machine-readable results;
- disclosure of assumptions and limitations; and
- a link to the corresponding VectorAmp article when published.

## License

Licensed under the [Apache License 2.0](LICENSE). Third-party datasets,
dependencies, and referenced works retain their respective licenses.

