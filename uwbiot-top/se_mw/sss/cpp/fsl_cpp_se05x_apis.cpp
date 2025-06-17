/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fsl_sss_se05x_apis.hpp>

#if SSS_HAVE_APPLET_SE05X_IOT

using namespace sss;
void *SESSION_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t SESSION_SE05X::create(
    sss_type_t subsystem, uint32_t application_id, sss_connection_type_t connection_type, void *connectionData)
{
    return sss_se05x_session_create(&this->ctx, subsystem, application_id, connection_type, connectionData);
}

sss_status_t SESSION_SE05X::open(
    sss_type_t subsystem, uint32_t application_id, sss_connection_type_t connection_type, void *connectionData)
{
    return sss_se05x_session_open(&this->ctx, subsystem, application_id, connection_type, connectionData);
}

sss_status_t SESSION_SE05X::prop_get_u32(uint32_t property, uint32_t *pValue)
{
    return sss_se05x_session_prop_get_u32(&this->ctx, property, pValue);
}

sss_status_t SESSION_SE05X::prop_get_au8(uint32_t property, uint8_t *pValue, size_t *pValueLen)
{
    return sss_se05x_session_prop_get_au8(&this->ctx, property, pValue, pValueLen);
}

void SESSION_SE05X::close()
{
    sss_se05x_session_close(&this->ctx);
}

void SESSION_SE05X::dodelete()
{
    sss_se05x_session_delete(&this->ctx);
}

void *KEYOBJ_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t KEYOBJ_SE05X::init(I_keystore *keyStore)
{
    return sss_se05x_key_object_init(&this->ctx, (sss_se05x_key_store_t *)keyStore->getCtx());
}

sss_status_t KEYOBJ_SE05X::allocate_handle(
    uint32_t keyId, sss_key_part_t keyPart, sss_cipher_type_t cipherType, size_t keyByteLenMax, uint32_t options)
{
    return sss_se05x_key_object_allocate_handle(&this->ctx, keyId, keyPart, cipherType, keyByteLenMax, options);
}

sss_status_t KEYOBJ_SE05X::get_handle(uint32_t keyId)
{
    return sss_se05x_key_object_get_handle(&this->ctx, keyId);
}

sss_status_t KEYOBJ_SE05X::set_user(uint32_t user, uint32_t options)
{
    return sss_se05x_key_object_set_user(&this->ctx, user, options);
}

sss_status_t KEYOBJ_SE05X::set_purpose(sss_mode_t purpose, uint32_t options)
{
    return sss_se05x_key_object_set_purpose(&this->ctx, purpose, options);
}

sss_status_t KEYOBJ_SE05X::set_access(uint32_t access, uint32_t options)
{
    return sss_se05x_key_object_set_access(&this->ctx, access, options);
}

sss_status_t KEYOBJ_SE05X::set_eccgfp_group(sss_eccgfp_group_t *group)
{
    return sss_se05x_key_object_set_eccgfp_group(&this->ctx, group);
}

sss_status_t KEYOBJ_SE05X::get_user(uint32_t *user)
{
    return sss_se05x_key_object_get_user(&this->ctx, user);
}

sss_status_t KEYOBJ_SE05X::get_purpose(sss_mode_t *purpose)
{
    return sss_se05x_key_object_get_purpose(&this->ctx, purpose);
}

sss_status_t KEYOBJ_SE05X::get_access(uint32_t *access)
{
    return sss_se05x_key_object_get_access(&this->ctx, access);
}

void KEYOBJ_SE05X::free()
{
    sss_se05x_key_object_free(&this->ctx);
}

void *KEYDERIVE_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t KEYDERIVE_SE05X::context_init(
    I_session *session, I_keyobj *keyObject, sss_algorithm_t algorithm, sss_mode_t mode)
{
    return sss_se05x_derive_key_context_init(&this->ctx,
        (sss_se05x_session_t *)session->getCtx(),
        (sss_se05x_object_t *)keyObject->getCtx(),
        algorithm,
        mode);
}

sss_status_t KEYDERIVE_SE05X::go(const uint8_t *saltData,
    size_t saltLen,
    const uint8_t *info,
    size_t infoLen,
    I_keyobj *derivedKeyObject,
    uint16_t deriveDataLen,
    uint8_t *hkdfOutput,
    size_t *hkdfOutputLen)
{
    return sss_se05x_derive_key_go(&this->ctx,
        saltData,
        saltLen,
        info,
        infoLen,
        (sss_se05x_object_t *)derivedKeyObject->getCtx(),
        deriveDataLen,
        hkdfOutput,
        hkdfOutputLen);
}

sss_status_t KEYDERIVE_SE05X::dh(I_keyobj *otherPartyKeyObject, I_keyobj *derivedKeyObject)
{
    return sss_se05x_derive_key_dh(&this->ctx,
        (sss_se05x_object_t *)otherPartyKeyObject->getCtx(),
        (sss_se05x_object_t *)derivedKeyObject->getCtx());
}

void KEYDERIVE_SE05X::context_free()
{
    sss_se05x_derive_key_context_free(&this->ctx);
}

void *KEYSTORE_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t KEYSTORE_SE05X::context_init(I_session *session)
{
    return sss_se05x_key_store_context_init(&this->ctx, (sss_se05x_session_t *)session->getCtx());
}

sss_status_t KEYSTORE_SE05X::allocate(uint32_t keyStoreId)
{
    return sss_se05x_key_store_allocate(&this->ctx, keyStoreId);
}

sss_status_t KEYSTORE_SE05X::save()
{
    return sss_se05x_key_store_save(&this->ctx);
}

sss_status_t KEYSTORE_SE05X::load()
{
    return sss_se05x_key_store_load(&this->ctx);
}

sss_status_t KEYSTORE_SE05X::set_key(
    I_keyobj *keyObject, const uint8_t *data, size_t dataLen, size_t keyBitLen, void *options, size_t optionsLen)
{
    return sss_se05x_key_store_set_key(
        &this->ctx, (sss_se05x_object_t *)keyObject->getCtx(), data, dataLen, keyBitLen, options, optionsLen);
}

sss_status_t KEYSTORE_SE05X::generate_key(I_keyobj *keyObject, size_t keyBitLen, void *options)
{
    return sss_se05x_key_store_generate_key(&this->ctx, (sss_se05x_object_t *)keyObject->getCtx(), keyBitLen, options);
}

sss_status_t KEYSTORE_SE05X::get_key(I_keyobj *keyObject, uint8_t *data, size_t *dataLen, size_t *pKeyBitLen)
{
    return sss_se05x_key_store_get_key(
        &this->ctx, (sss_se05x_object_t *)keyObject->getCtx(), data, dataLen, pKeyBitLen);
}

sss_status_t KEYSTORE_SE05X::open_key(I_keyobj *keyObject)
{
    return sss_se05x_key_store_open_key(&this->ctx, (sss_se05x_object_t *)keyObject->getCtx());
}

sss_status_t KEYSTORE_SE05X::freeze_key(I_keyobj *keyObject)
{
    return sss_se05x_key_store_freeze_key(&this->ctx, (sss_se05x_object_t *)keyObject->getCtx());
}

sss_status_t KEYSTORE_SE05X::erase_key(I_keyobj *keyObject)
{
    return sss_se05x_key_store_erase_key(&this->ctx, (sss_se05x_object_t *)keyObject->getCtx());
}

void KEYSTORE_SE05X::context_free()
{
    sss_se05x_key_store_context_free(&this->ctx);
}

void *ASYM_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t ASYM_SE05X::context_init(
    I_session *session, I_keyobj *keyObject, sss_algorithm_t algorithm, sss_mode_t mode)
{
    return sss_se05x_asymmetric_context_init(&this->ctx,
        (sss_se05x_session_t *)session->getCtx(),
        (sss_se05x_object_t *)keyObject->getCtx(),
        algorithm,
        mode);
}

sss_status_t ASYM_SE05X::encrypt(const uint8_t *srcData, size_t srcLen, uint8_t *destData, size_t *destLen)
{
    return sss_se05x_asymmetric_encrypt(&this->ctx, srcData, srcLen, destData, destLen);
}

sss_status_t ASYM_SE05X::decrypt(const uint8_t *srcData, size_t srcLen, uint8_t *destData, size_t *destLen)
{
    return sss_se05x_asymmetric_decrypt(&this->ctx, srcData, srcLen, destData, destLen);
}

sss_status_t ASYM_SE05X::sign_digest(uint8_t *digest, size_t digestLen, uint8_t *signature, size_t *signatureLen)
{
    return sss_se05x_asymmetric_sign_digest(&this->ctx, digest, digestLen, signature, signatureLen);
}

sss_status_t ASYM_SE05X::verify_digest(uint8_t *digest, size_t digestLen, uint8_t *signature, size_t signatureLen)
{
    return sss_se05x_asymmetric_verify_digest(&this->ctx, digest, digestLen, signature, signatureLen);
}

void ASYM_SE05X::context_free()
{
    sss_se05x_asymmetric_context_free(&this->ctx);
}

void *SYMM_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t SYMM_SE05X::context_init(
    I_session *session, I_keyobj *keyObject, sss_algorithm_t algorithm, sss_mode_t mode)
{
    return sss_se05x_symmetric_context_init(&this->ctx,
        (sss_se05x_session_t *)session->getCtx(),
        (sss_se05x_object_t *)keyObject->getCtx(),
        algorithm,
        mode);
}

sss_status_t SYMM_SE05X::one_go(uint8_t *iv, size_t ivLen, const uint8_t *srcData, uint8_t *destData, size_t dataLen)
{
    return sss_se05x_cipher_one_go(&this->ctx, iv, ivLen, srcData, destData, dataLen);
}

sss_status_t SYMM_SE05X::one_go_v2(
    uint8_t *iv, size_t ivLen, const uint8_t *srcData, const size_t srcLen, uint8_t *destData, size_t *pDataLen)
{
    return sss_se05x_cipher_one_go_v2(&this->ctx, iv, ivLen, srcData, srcLen, destData, pDataLen);
}

sss_status_t SYMM_SE05X::init(uint8_t *iv, size_t ivLen)
{
    return sss_se05x_cipher_init(&this->ctx, iv, ivLen);
}

sss_status_t SYMM_SE05X::update(const uint8_t *srcData, size_t srcLen, uint8_t *destData, size_t *destLen)
{
    return sss_se05x_cipher_update(&this->ctx, srcData, srcLen, destData, destLen);
}

sss_status_t SYMM_SE05X::finish(const uint8_t *srcData, size_t srcLen, uint8_t *destData, size_t *destLen)
{
    return sss_se05x_cipher_finish(&this->ctx, srcData, srcLen, destData, destLen);
}

sss_status_t SYMM_SE05X::crypt_ctr(const uint8_t *srcData,
    uint8_t *destData,
    size_t size,
    uint8_t *initialCounter,
    uint8_t *lastEncryptedCounter,
    size_t *szLeft)
{
    return sss_se05x_cipher_crypt_ctr(
        &this->ctx, srcData, destData, size, initialCounter, lastEncryptedCounter, szLeft);
}

void SYMM_SE05X::context_free()
{
    sss_se05x_symmetric_context_free(&this->ctx);
}

void *AEAD_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t AEAD_SE05X::context_init(
    I_session *session, I_keyobj *keyObject, sss_algorithm_t algorithm, sss_mode_t mode)
{
    return sss_se05x_aead_context_init(&this->ctx,
        (sss_se05x_session_t *)session->getCtx(),
        (sss_se05x_object_t *)keyObject->getCtx(),
        algorithm,
        mode);
}

sss_status_t AEAD_SE05X::one_go(const uint8_t *srcData,
    uint8_t *destData,
    size_t size,
    uint8_t *nonce,
    size_t nonceLen,
    const uint8_t *aad,
    size_t aadLen,
    uint8_t *tag,
    size_t *tagLen)
{
    return sss_se05x_aead_one_go(&this->ctx, srcData, destData, size, nonce, nonceLen, aad, aadLen, tag, tagLen);
}

sss_status_t AEAD_SE05X::init(uint8_t *nonce, size_t nonceLen, size_t tagLen, size_t aadLen, size_t payloadLen)
{
    return sss_se05x_aead_init(&this->ctx, nonce, nonceLen, tagLen, aadLen, payloadLen);
}

sss_status_t AEAD_SE05X::update_aad(const uint8_t *aadData, size_t aadDataLen)
{
    return sss_se05x_aead_update_aad(&this->ctx, aadData, aadDataLen);
}

sss_status_t AEAD_SE05X::update(const uint8_t *srcData, size_t srcLen, uint8_t *destData, size_t *destLen)
{
    return sss_se05x_aead_update(&this->ctx, srcData, srcLen, destData, destLen);
}

sss_status_t AEAD_SE05X::finish(
    const uint8_t *srcData, size_t srcLen, uint8_t *destData, size_t *destLen, uint8_t *tag, size_t *tagLen)
{
    return sss_se05x_aead_finish(&this->ctx, srcData, srcLen, destData, destLen, tag, tagLen);
}

void AEAD_SE05X::context_free()
{
    sss_se05x_aead_context_free(&this->ctx);
}

void *MAC_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t MAC_SE05X::context_init(
    I_session *session, I_keyobj *keyObject, sss_algorithm_t algorithm, sss_mode_t mode)
{
    return sss_se05x_mac_context_init(&this->ctx,
        (sss_se05x_session_t *)session->getCtx(),
        (sss_se05x_object_t *)keyObject->getCtx(),
        algorithm,
        mode);
}

sss_status_t MAC_SE05X::one_go(const uint8_t *message, size_t messageLen, uint8_t *mac, size_t *macLen)
{
    return sss_se05x_mac_one_go(&this->ctx, message, messageLen, mac, macLen);
}

sss_status_t MAC_SE05X::init()
{
    return sss_se05x_mac_init(&this->ctx);
}

sss_status_t MAC_SE05X::update(const uint8_t *message, size_t messageLen)
{
    return sss_se05x_mac_update(&this->ctx, message, messageLen);
}

sss_status_t MAC_SE05X::finish(uint8_t *mac, size_t *macLen)
{
    return sss_se05x_mac_finish(&this->ctx, mac, macLen);
}

void MAC_SE05X::context_free()
{
    sss_se05x_mac_context_free(&this->ctx);
}

void *MD_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t MD_SE05X::context_init(I_session *session, sss_algorithm_t algorithm, sss_mode_t mode)
{
    return sss_se05x_digest_context_init(&this->ctx, (sss_se05x_session_t *)session->getCtx(), algorithm, mode);
}

sss_status_t MD_SE05X::one_go(const uint8_t *message, size_t messageLen, uint8_t *digest, size_t *digestLen)
{
    return sss_se05x_digest_one_go(&this->ctx, message, messageLen, digest, digestLen);
}

sss_status_t MD_SE05X::init()
{
    return sss_se05x_digest_init(&this->ctx);
}

sss_status_t MD_SE05X::update(const uint8_t *message, size_t messageLen)
{
    return sss_se05x_digest_update(&this->ctx, message, messageLen);
}

sss_status_t MD_SE05X::finish(uint8_t *digest, size_t *digestLen)
{
    return sss_se05x_digest_finish(&this->ctx, digest, digestLen);
}

void MD_SE05X::context_free()
{
    sss_se05x_digest_context_free(&this->ctx);
}

void *RNG_SE05X::getCtx()
{
    return (void *)&this->ctx;
}
sss_status_t RNG_SE05X::context_init(I_session *session)
{
    return sss_se05x_rng_context_init(&this->ctx, (sss_se05x_session_t *)session->getCtx());
}

sss_status_t RNG_SE05X::get_random(uint8_t *random_data, size_t dataLen)
{
    return sss_se05x_rng_get_random(&this->ctx, random_data, dataLen);
}

sss_status_t RNG_SE05X::context_free()
{
    return sss_se05x_rng_context_free(&this->ctx);
}

#endif /* SSS_HAVE_APPLET_SE05X_IOT */
