# c-dvar - D-Bus Variant Type-System

## CHANGES WITH X.Y.Z:

        * Fix a var-arg error in the test-suite.

        Contributions from: David Rheinsberg, Sinkevich Artem

        - XYZ, YYYY-MM-DD

## CHANGES WITH 1.2.0:

        * Fix the variant-reader to use aliasing-safe accessors. This allows
          parallel use of the variant-buffers with possibly aliasing pointers.

        Contributions from: David Rheinsberg, Kostadin Shishmanov

        - Dußlingen, 2025-06-16

## CHANGES WITH 1.1.0:

        * Update the c-stdaux dependency to provide the new build variables
          and thus allow linking through pkg-config.

        Contributions from: David Rheinsberg

        - Dußlingen, 2023-12-12

## CHANGES WITH 1.0.0:

        * Initial release of c-dvar.

        Contributions from: Adrian Szyndela, David Rheinsberg, Evgeny
                            Vereshchagin, Tom Gundersen

        - Brno, 2022-06-22
