# Systems

In the following, we present a list of sources we used for the evaluated systems.  Note that `Automotive03-05` cannot be published. Each directory contains feature model(s) corresponding to the subject system. KConfig and CDL feature models are grouped for better readability.

* Knüppel et al. \[[KTM+:SE18](https://dl.acm.org/doi/abs/10.1145/3106237.3106252)\]
  * [CDL](https://github.com/AlexanderKnueppel/is-there-a-mismatch/tree/master/Data/LargeFeatureModels/CDL)
  * [kConfig](https://github.com/AlexanderKnueppel/is-there-a-mismatch/tree/master/Data/LargeFeatureModels/KConfig)
  * [Automotive02](https://github.com/AlexanderKnueppel/is-there-a-mismatch/tree/master/Data/LargeFeatureModels)
* Pett et al. \[[PKRT:VaMoS21](https://dl.acm.org/doi/abs/10.1145/3442391.3442410)\]
  * [BusyBox](https://github.com/TUBS-ISF/Stability-of-Productline-Sampling)
* [FeatureIDE Examples](https://github.com/FeatureIDE/FeatureIDE)
  * [FinancialServices](https://github.com/FeatureIDE/FeatureIDE/tree/develop/plugins/de.ovgu.featureide.examples/featureide_examples/FeatureModels/FinancialServices01)
  * [Automotive01](https://github.com/FeatureIDE/FeatureIDE/tree/develop/plugins/de.ovgu.featureide.examples/featureide_examples/FeatureModels/Automotive01)
  * [BerkeleyDB](https://github.com/FeatureIDE/FeatureIDE/tree/develop/plugins/de.ovgu.featureide.examples/featureide_examples/FeatureModels/BerkeleyDB)
* Automotive 03-05 (confidential and, thus, not included)


### Variability Models
|Variability Model |Version |Domain            |Number of Models|Number of Features|Number of Constraints|
|------------------|--------------|------------------|--------------|----------------|-------------------|
|Automotive01      |v1 |Automotive        |1             |2513            |2833               |
|Automotive02      |v4 |Automotive        |1             |18,616     |1,369           |
|BusyBox           |Version from 2010-05-02 |Operating System  |1            |631         |681            |
|CDL               |n.a. (published 2017-06) |Operating System  |116           |1178–1,408       |816–956            |
|BerkeleyDB        |n.a. (published 2011-06) |Database          |1             |76              |20                 |
|Financial Services|v10 |Financial Services|1            |771        |1,080         |
|KConfig axTLS            |n.a. (published 2017-06) | Operating System | 1             |96         |14             |
|KConfig uCLibc           |n.a. (published 2017-06) |Operating System  |1             |313         |56            |
|KConfig uClinux-base          |n.a. (published 2017-06) |Operating System  |1             |380         |3,455            |
|KConfig Embtoolkit          |n.a. (published 2017-06) |Operating System  |1             |1,179         |323            |
|KConfig uClinux-Distribution          |n.a. (published 2017-06) |Operating System  |1             |1,580         |197            |
|KConfig Embtoolkit          |n.a. (published 2017-06) |Operating System  |1             |1,179         |323            |
|KConfig Linux    |2.6.33.3 |Operating System  |1             |6,467            |3,545               |
