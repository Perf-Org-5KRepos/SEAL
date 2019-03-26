// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "seal/randomgen.h"
#include "seal/plaintext.h"
#include "seal/memorymanager.h"
#include "seal/util/common.h"
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <random>

namespace seal
{
    /**
    Class to store a secret key.

    @par Thread Safety
    In general, reading from SecretKey is thread-safe as long as no other thread
    is concurrently mutating it. This is due to the underlying data structure
    storing the secret key not being thread-safe.

    @see KeyGenerator for the class that generates the secret key.
    @see PublicKey for the class that stores the public key.
    @see RelinKeys for the class that stores the relinearization keys.
    @see GaloisKeys for the class that stores the Galois keys.
    */
    class SecretKey
    {
        friend class KeyGenerator;

    public:
        /**
        Creates an empty secret key.
        */
        SecretKey() = default;

        /**
        Creates a new SecretKey by copying an old one.

        @param[in] copy The SecretKey to copy from
        */
        SecretKey(const SecretKey &copy)
        {
            // Note: sk_ is at this point initialized to use a custom (new)
            // memory pool with the `clear_on_destruction' property. Now use
            // Plaintext::operator =(const Plaintext &) to copy over the data.
            // This is very important to do right, otherwise newly created
            // SecretKey may use a normal memory pool obtained from
            // MemoryManager::GetPool() with currently active profile (MMProf).
            sk_ = copy.sk_;
        }

        /**
        Destroys the SecretKey object.
        */
        ~SecretKey() = default;

        /**
        Creates a new SecretKey by moving an old one.

        @param[in] source The SecretKey to move from
        */
        SecretKey(SecretKey &&source) = default;

        /**
        Copies an old SecretKey to the current one.

        @param[in] assign The SecretKey to copy from
        */
        SecretKey &operator =(const SecretKey &assign)
        {
            sk_ = assign.sk_;
            return *this;
        }

        /**
        Moves an old SecretKey to the current one.

        @param[in] assign The SecretKey to move from
        */
        SecretKey &operator =(SecretKey &&assign) = default;

        /**
        Returns a reference to the underlying polynomial.
        */
        inline auto &data() noexcept
        {
            return sk_;
        }

        /**
        Returns a const reference to the underlying polynomial.
        */
        inline auto &data() const noexcept
        {
            return sk_;
        }

        /**
        Check whether the current SecretKey is valid for a given SEALContext. If
        the given SEALContext is not set, the encryption parameters are invalid,
        or the SecretKey data does not match the SEALContext, this function returns
        false. Otherwise, returns true.

        @param[in] context The SEALContext
        */
        inline bool is_valid_for(std::shared_ptr<const SEALContext> context) const
        {
            // Check metadata
            if (!is_metadata_valid_for(context))
            {
                return false;
            }

            // Check the data
            return sk_.is_valid_for(std::move(context));
        }

        /**
        Check whether the current SecretKey is valid for a given SEALContext. If
        the given SEALContext is not set, the encryption parameters are invalid,
        or the SecretKey data does not match the SEALContext, this function returns
        false. Otherwise, returns true. This function only checks the metadata
        and not the secret key data itself.

        @param[in] context The SEALContext
        */
        inline bool is_metadata_valid_for(std::shared_ptr<const SEALContext> context) const
        {
            // Verify parameters
            if (!context || !context->parameters_set())
            {
                return false;
            }
            auto parms_id = context->key_parms_id();
            return sk_.is_metadata_valid_for(std::move(context)) &&
                sk_.is_ntt_form() && sk_.parms_id() == parms_id;
        }

        /**
        Saves the SecretKey to an output stream. The output is in binary format
        and not human-readable. The output stream must have the "binary" flag set.

        @param[in] stream The stream to save the SecretKey to
        @throws std::exception if the plaintext could not be written to stream
        */
        inline void save(std::ostream &stream) const
        {
            sk_.save(stream);
        }

        /**
        Loads a SecretKey from an input stream overwriting the current SecretKey.
        No checking of the validity of the SecretKey data against encryption
        parameters is performed. This function should not be used unless the
        SecretKey comes from a fully trusted source.

        @param[in] stream The stream to load the SecretKey from
        @throws std::exception if a valid SecretKey could not be read from stream
        */
        inline void unsafe_load(std::istream &stream)
        {
            sk_.unsafe_load(stream);
        }

        /**
        Loads a SecretKey from an input stream overwriting the current SecretKey.
        The loaded SecretKey is verified to be valid for the given SEALContext.

        @param[in] context The SEALContext
        @param[in] stream The stream to load the SecretKey from
        @throws std::invalid_argument if the context is not set or encryption
        parameters are not valid
        @throws std::exception if a valid SecretKey could not be read from stream
        @throws std::invalid_argument if the loaded SecretKey is invalid for the
        context
        */
        inline void load(std::shared_ptr<SEALContext> context,
            std::istream &stream)
        {
            unsafe_load(stream);
            if (!is_valid_for(std::move(context)))
            {
                throw std::invalid_argument("SecretKey data is invalid");
            }
        }

        /**
        Returns a reference to parms_id.

        @see EncryptionParameters for more information about parms_id.
        */
        inline auto &parms_id() noexcept
        {
            return sk_.parms_id();
        }

        /**
        Returns a const reference to parms_id.

        @see EncryptionParameters for more information about parms_id.
        */
        inline auto &parms_id() const noexcept
        {
            return sk_.parms_id();
        }

        /**
        Returns the currently used MemoryPoolHandle.
        */
        inline MemoryPoolHandle pool() const noexcept
        {
            return sk_.pool();
        }

    private:
        // We use a fresh memory pool with `clear_on_destruction' enabled.
        Plaintext sk_{ MemoryManager::GetPool(mm_prof_opt::FORCE_NEW, true) };
    };
}
