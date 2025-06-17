/*
 * Copyright 2018,2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ***************************************************************** */
/* ***************************************************************** */

/* @defgroup sss SSS
 *
 * @brief SSS
 *
 * - See @ref sss_session
 * - See @ref sss_key_store
 * - See @ref sss_key_object
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_types SSS Types
 *
 * @brief SSS Types
 *
 */

/** @defgroup sss_session Session
 *
 * @brief Manage session
 *
 */

/** @ingroup sss_session
 *
 * @defgroup sss_sscp_session sscp : Session
 *
 * @brief SSCP Client Session management
 */

/* @ingroup sscp_session
 *
 *  @defgroup sss_a71ch_session A71CH : Session
 *
 *  @brief Session object for A71CH Secure Element
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_key_store Key Store
 *
 * @brief Secure storage for keys and certificates.
 */

/** @ingroup sss_key_store
 *
 * @defgroup sss_sscp_keystore sscp : Key Store
 *
 * @brief SSCP Client key store
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_key_object Key Object
 *
 * @brief Low level iota of key/certificates in \c SSS domain.
 */

/** @ingroup sss_key_object
 *
 * @defgroup sss_sscp_keyobj sscp : Key Object
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_symmetric Symmetric Crypto
 *
 * @brief Symmetric cryptographic operations like \c AES / \c DES/etc.
 */

/** @ingroup sss_crypto_symmetric
 *
 * @defgroup sss_sscp_symm sscp Symmetric Crypto
 *
 * @brief Asymmetric cryptographic operations like \c RSA / \c ECC/etc.
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_asymmetric Asymmetric Crypto
 *
 * @brief Asymmetric cryptographic operations like \c RSA / \c ECC/etc.
 */

/** @ingroup sss_crypto_asymmetric
 *
 * @defgroup sss_sscp_asym sscp Asymmetric Crypto
 *
 * @brief @ref sss_crypto_asymmetric for sscp.
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_aead AEAD
 *
 *  @brief Authenticated Encryption with Additional Data
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_tunnel Tunnel
 *
 *  @brief Tunnel session
 */

/* ***************************************************************** */ /* ***************************************************************** */

/** @defgroup sss_crypto_digest Digest
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_mac Mac
 */

/** @defgroup sss_policy Policy
 * 
 * Policies to restrict and control sessions and objects.
 */
/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_tunnelling Tunneling
 *
 * @brief Tunnel for SCP
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_crypto_derive_key Derive Key
 *
 * @brief Derive Key
 *
 * Operations that use mutiple keys from multiple
 * security systems and dervive
 * and output key, e.g. Diffie Hellman exchange.
 *
 */

/** @ingroup sss_crypto_derive_key
 *
 * @defgroup sss_sscp_keyderive SSCP Key Derive
 *
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @ingroup sss_crypto_mac
 *
 * @defgroup sss_sscp_mac SSCP MAC
 *
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup sss_rng Random Number Generator
 *
 *
 */

/** @ingroup sss_rng
 *
 * @defgroup sss_sscp_rng RNG
 *
 */

/* ***************************************************************** */
/* ***************************************************************** */

/** @defgroup se05x_attest SE05x Attestation
 *
 *
 */

/** @defgroup se05x_other Miscellaneous SE05x types and APIs
 *
 *
 */
/* ***************************************************************** */
/* ***************************************************************** */

/* @ingroup sss
 *
 * @defgroup sscp SSCP
 */

/** @ingroup sscp
 *
 * @defgroup sss_a71ch SSS : A71CH
 *
 * @brief A71CH
 */

/** @ingroup sscp
 *
 * @defgroup sss_se050 SSS : SE050
 *
 * @brief SE050
 */

/** @ingroup sscp
 *
 * @defgroup sss_sscp_cmds SSCP Commands
 *
 * @brief SSCP Commmands
 */

/** @ingroup sscp
 *
 * @defgroup sss_sscp_a71ch SSCP : A71CH
 *
 * @brief Connecting to A71CH
 */

/** @defgroup ax_mbed_tls mbedTLS ALT
 *
 * mbedTLS ALT implementation.
 *
 */

/** @defgroup sss_se05x_session SE05x session
 *  @ingroup sss_session
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_keystore SE05x session
 *  @ingroup sss_key_store
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_keyobj SE05x session
 *  @ingroup sss_key_object
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_symm SE05x session
 *  @ingroup sss_crypto_symmetric
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_asym SE05x session
 *  @ingroup sss_crypto_asymmetric
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_keyderive SE05x session
 *  @ingroup sss_crypto_derive_key
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_aead SE05x session
 *  @ingroup sss_crypto_aead
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_tunnel SE05x session
 *  @ingroup sss_crypto_tunnel
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_tunnel SE05x session
 *  @ingroup sss_crypto_tunnel
 *
 *  @brief Manage session
 */

/** @defgroup sss_sw_se05x SE05x session
 *  @ingroup sss_types
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_rng SE05x session
 *  @ingroup sss_rng
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_md SE05x session
 *  @ingroup sss_crypto_digest
 *
 *  @brief Manage session
 */

/** @defgroup sss_se05x_mac SE05x session
 *  @ingroup sss_crypto_mac
 *
 *  @brief Manage session
 */
