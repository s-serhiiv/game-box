#include "game_center_provier.h"
#include "push_notifications_provider.h"
#include "store_provider.h"

#if defined(__WINOS__)

namespace game
{
    std::shared_ptr<game_center_provier> game_center_provier::m_instance = nullptr;

    game_center_provier::game_center_provier() = default;
    game_center_provier::~game_center_provier() = default;

    std::shared_ptr<game_center_provier> game_center_provier::shared_instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_shared<game_center_provier>();
        }
        return m_instance;
    }

    void game_center_provier::authenticate() { }
    void game_center_provier::rate_app() { }
    void game_center_provier::open_drifters_leaderboard() { }
    void game_center_provier::report_drifring_score(i32) { }

    std::shared_ptr<push_notifications_provider> push_notifications_provider::m_instance = nullptr;

    push_notifications_provider::push_notifications_provider() = default;
    push_notifications_provider::~push_notifications_provider() = default;

    std::shared_ptr<push_notifications_provider> push_notifications_provider::shared_instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_shared<push_notifications_provider>();
        }
        return m_instance;
    }

    void push_notifications_provider::authenticate() { }
    void push_notifications_provider::schedule_day_notifications() { }
    void push_notifications_provider::schedule_offline_notifications() { }

    const i32 store_provider::k_no_ads_product_id = 1;
    const i32 store_provider::k_cash_pack_1_product_id = 2;
    const i32 store_provider::k_cash_pack_2_product_id = 3;
    const i32 store_provider::k_cash_pack_3_product_id = 4;
    const i32 store_provider::k_vip_subscription_product_id = 5;

    std::shared_ptr<store_provider> store_provider::m_instance = nullptr;

    store_provider::store_provider() = default;
    store_provider::~store_provider() = default;

    std::shared_ptr<store_provider> store_provider::shared_instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_shared<store_provider>();
        }
        return m_instance;
    }

    void store_provider::request_products() { }

    void store_provider::buy_no_ads_product(const std::function<void(bool)>& callback)
    {
        m_on_purchase_no_ads = callback;
        if (callback) callback(false);
    }

    void store_provider::buy_small_cash_pack(const std::function<void(bool)>& callback)
    {
        m_on_purchase_small_cash_pack = callback;
        if (callback) callback(false);
    }

    void store_provider::buy_medium_cash_pack(const std::function<void(bool)>& callback)
    {
        m_on_purchase_medium_cash_pack = callback;
        if (callback) callback(false);
    }

    void store_provider::buy_big_cash_pack(const std::function<void(bool)>& callback)
    {
        m_on_purchase_big_cash_pack = callback;
        if (callback) callback(false);
    }

    void store_provider::buy_vip_subscription(const std::function<void(bool)>& callback)
    {
        m_on_purchase_vip_subscription = callback;
        if (callback) callback(false);
    }

    void store_provider::restore_products()
    {
        if (m_on_purchases_restored) m_on_purchases_restored(0);
    }

    void store_provider::set_on_puchases_restored_callback(const std::function<void(i32)>& callback)
    {
        m_on_purchases_restored = callback;
    }

    std::function<void(i32)> store_provider::get_on_puchases_restored_callback() const
    {
        return m_on_purchases_restored;
    }

    void store_provider::set_on_subscription_status_changed_callback(const std::function<void(bool)>& callback)
    {
        m_on_subscription_status_changed = callback;
    }

    std::function<void(bool)> store_provider::get_on_subscription_status_changed_callback() const
    {
        return m_on_subscription_status_changed;
    }

    std::function<void(bool)> store_provider::get_on_puchase_no_ads_callback() const
    {
        return m_on_purchase_no_ads;
    }

    std::function<void(bool)> store_provider::get_on_puchase_small_cash_pack_callback() const
    {
        return m_on_purchase_small_cash_pack;
    }

    std::function<void(bool)> store_provider::get_on_puchase_medium_cash_pack_callback() const
    {
        return m_on_purchase_medium_cash_pack;
    }

    std::function<void(bool)> store_provider::get_on_puchase_big_cash_pack_callback() const
    {
        return m_on_purchase_big_cash_pack;
    }

    std::function<void(bool)> store_provider::get_on_puchase_vip_subscription_callback() const
    {
        return m_on_purchase_vip_subscription;
    }

    bool store_provider::is_no_ads_product_bought() const
    {
        return true;
    }
}

#endif
