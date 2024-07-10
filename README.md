# A collection of community-maintained components for Genode

This repository hosts Genode components that do not fall in the narrow scope of
the [Genode OS Framework](http://genode.org), e.g, ported applications,
libraries, and games.

To use it, you first need to obtain a clone of Genode:

```sh
$ git clone https://github.com/genodelabs/genode.git genode
```

Now, clone the `genode-world.git` repository to `genode/repos/world`:

```sh
$ git clone https://github.com/genodelabs/genode-world.git genode/repos/world
```

By placing the `world` repository under the `repos/` directory, Genode's tools
will automatically incorporate the ports provided by the `world` repository.

For building software of the `world` repository, the build-directory
configuration `etc/build.conf` must be extended with the following line:

```sh
$ REPOSITORIES += $(GENODE_DIR)/repos/world
```

# Note of caution

In contrast to the components found in the mainline Genode repository, the
components within the `world` repository are not subjected to the regular
quality-assurance measures of Genode Labs. Hence, problems are to be expected.
If you encounter bugs, build problems, or stability issues, please report them
to the [issue tracker](https://github.com/genodelabs/genode-world/issues) 
or the [Genode mailing list](http://genode.org/community/mailing-lists).
