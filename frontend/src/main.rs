use wasm_bindgen_futures::spawn_local;
use gloo_net::http::Request;
use web_sys::console;
use yew::prelude::*;

fn determine_api_base() -> String {
    if let Some(window) = web_sys::window() {
        if let Ok(origin) = window.location().origin() {
            if origin.contains("127.0.0.1:8080") || origin.contains("localhost:8080") {
                return "http://127.0.0.1:3000".to_string();
            }
            return origin;
        }
    }
    "http://127.0.0.1:3000".to_string()
}

#[function_component(App)]
fn app() -> Html {
    let query = use_state(|| "latest technology news".to_string());
    let result = use_state(|| String::new());
    let loading = use_state(|| false);
    let error = use_state(|| None as Option<String>);

    // helper to fetch results
    let fetch = {
        let query = query.clone();
        let result = result.clone();
        let loading = loading.clone();
        let error = error.clone();
        Callback::from(move |_| {
            let q = (*query).clone();
            let result = result.clone();
            let loading = loading.clone();
            let error = error.clone();

            loading.set(true);
            error.set(None);
            result.set(String::new());

            // minimal encoding: replace spaces with + (server decodes it)
            let q_enc = q.replace(' ', "+");
            let base = determine_api_base();
            let url = format!("{}/api/search?q={}", base, q_enc);
            console::log_1(&format!("Fetching: {}", url).into());

            spawn_local(async move {
                match Request::get(&url).send().await {
                    Ok(resp) => {
                        let status = resp.status();
                        match resp.text().await {
                            Ok(text) => {
                                console::log_1(&format!("Response {} bytes (status {}): {}", text.len(), status, status).into());
                                result.set(text);
                                loading.set(false);
                            }
                            Err(e) => {
                                let msg = format!("Failed reading response: {}", e);
                                console::error_1(&msg.clone().into());
                                error.set(Some(msg));
                                loading.set(false);
                            }
                        }
                    }
                    Err(e) => {
                        let msg = format!("Request failed: {}", e);
                        console::error_1(&msg.clone().into());
                        error.set(Some(msg));
                        loading.set(false);
                    }
                }
            });
        })
    };

    // fetch on mount
    {
        let fetch = fetch.clone();
        use_effect_with((), move |_| {
            fetch.emit(());
            || {}
        });
    }

    let oninput = {
        let query = query.clone();
        Callback::from(move |e: InputEvent| {
            let input: web_sys::HtmlInputElement = e.target_unchecked_into();
            query.set(input.value());
        })
    };

    let onsubmit = {
        let fetch = fetch.clone();
        Callback::from(move |e: SubmitEvent| {
            e.prevent_default();
            fetch.emit(());
        })
    };

    html! {
        <div style="max-width: 800px; margin: 2rem auto; font-family: sans-serif;">
            <h1>{"Perplexity Search"}</h1>
            <form onsubmit={onsubmit} style="display:flex; gap: 0.5rem;">
                <input
                    type="text"
                    value={(*query).clone()}
                    oninput={oninput}
                    placeholder="Enter your query"
                    style="flex:1; padding: 0.5rem; font-size: 1rem;"/>
                <button type="submit" style="padding: 0.5rem 1rem; font-size: 1rem;">{"Search"}</button>
            </form>
            <div style="margin-top:1rem;">
                {
                    if *loading { html!{ <p>{"Loading..."}</p> } }
                    else if let Some(err) = &*error { html!{ <p style="color:red;">{err}</p> } }
                    else if !result.is_empty() { html!{ <pre style="white-space: pre-wrap; background:#f7f7f7; padding:1rem; border:1px solid #ddd;">{ (*result).clone() }</pre> } }
                    else { html!{ <p style="color:#666;">{"Ready. Enter a query and click Search."}</p> } }
                }
            </div>
            <p style="color:#666; font-size:0.9rem; margin-top:1rem;">{"Note: set PERPLEXITY_API_KEY in the backend environment and ensure './simpcurl' is built."}</p>
        </div>
    }
}

fn main() {
    // Better error messages if the WASM panics
    console_error_panic_hook::set_once();

    let document = web_sys::window().unwrap().document().unwrap();
    let root = document.get_element_by_id("root").expect("#root element not found in index.html");
    yew::Renderer::<App>::with_root(root).render();
}
