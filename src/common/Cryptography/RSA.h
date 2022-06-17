/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Define.h"
#include <openssl/objects.h>
#include <openssl/rsa.h>
#include <array>
#include <memory>
#include <string>
#include <type_traits>

class BigNumber;

namespace Trinity
{
namespace Crypto
{
class TC_COMMON_API RSA
{
public:
    class TC_COMMON_API DigestGenerator
    {
    public:
        struct EVP_MD_Deleter
        {
            void operator()(EVP_MD* md) const;
        };

        virtual ~DigestGenerator() = default;
        virtual std::unique_ptr<EVP_MD, EVP_MD_Deleter> GetGenerator() const = 0;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        virtual OSSL_LIB_CTX* GetLib() const = 0;
        virtual std::unique_ptr<OSSL_PARAM[]> GetParams() const = 0;
#else
        virtual void PostInitCustomizeContext(EVP_MD_CTX* ctx) = 0;
#endif
    };

    class TC_COMMON_API SHA256 : public DigestGenerator
    {
    public:
        std::unique_ptr<EVP_MD, EVP_MD_Deleter> GetGenerator() const override;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        OSSL_LIB_CTX* GetLib() const override;
        std::unique_ptr<OSSL_PARAM[]> GetParams() const override;
#else
        void PostInitCustomizeContext(EVP_MD_CTX* ctx) override;
#endif
    };

    class TC_COMMON_API HMAC_SHA256 : public DigestGenerator
    {
    public:
        explicit HMAC_SHA256(uint8 const* key, size_t keyLength) : _key(key), _keyLength(keyLength) { }

        std::unique_ptr<EVP_MD, EVP_MD_Deleter> GetGenerator() const override;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        OSSL_LIB_CTX* GetLib() const override;
        std::unique_ptr<OSSL_PARAM[]> GetParams() const override;
#else
        void PostInitCustomizeContext(EVP_MD_CTX* ctx) override;
#endif

    private:
        uint8 const* _key;
        size_t _keyLength;
    };

    RsaSignature();
    RsaSignature(RsaSignature const& other);
    RsaSignature(RsaSignature&& other) noexcept;
    ~RsaSignature();

    RsaSignature& operator=(RsaSignature const& right);
    RsaSignature& operator=(RsaSignature&& right) noexcept;

    bool LoadKeyFromFile(std::string const& fileName);

    bool LoadKeyFromString(std::string const& keyPem);

    template <std::size_t N>
    bool Sign(std::array<uint8, N> const& message, DigestGenerator& generator, std::vector<uint8>& output)
    {
        return Sign(HashTag::value, dataHash, dataHashLength, output);
    }

    template <std::size_t N, typename HashTag>
    bool Sign(std::array<uint8, N> const& dataHash, uint8* output, HashTag)
    {
        return Sign(HashTag::value, dataHash.data(), dataHash.size(), output);
    }

private:
    template <typename KeyTag>
    bool Encrypt(uint8 const* data, std::size_t dataLength, uint8* output, int32 paddingType);

    bool Sign(int32 hashType, uint8 const* dataHash, std::size_t dataHashLength, uint8* output);

    RSA(RSA const& rsa) = delete;
    RSA& operator=(RSA const& rsa) = delete;

    ::RSA* _rsa;
};
}
}
